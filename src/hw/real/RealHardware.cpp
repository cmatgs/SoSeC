#include "hw/RealHardware.hpp"
#include <algorithm>
#include <string>

// ----- Kleiner Adapter, falls notwendig -----
// Wenn dein RelayController NICHT von IRelays erbt,
// adaptieren wir ihn hier transparent auf IRelays.
// Entfernen, wenn RelayController bereits IRelays implementiert.
class RelaysAdapter : public IRelays {
public:
    explicit RelaysAdapter(RelayController& rc) : rc_(rc) {}
    bool set(size_t idx, bool on) override { return rc_.set(idx, on, nullptr); }
    bool get(size_t idx, bool* on) override { return rc_.get(idx, on, nullptr); }
    bool toggle(size_t idx) override { return rc_.toggle(idx, nullptr); }
    bool setAll(bool on) override { return rc_.setAll(on, nullptr); }
    size_t count() const override { return rc_.count(); }
private:
    RelayController& rc_;
};

static std::optional<RelaysAdapter> g_relays_adapter; // Lebenszeit an RealHardware binden

// ----- LED Farbkodierung: 0..1 -> Grün bis Rot -----
void RealHardware::SeverityToRGB(double sev01, uint8_t& r, uint8_t& g, uint8_t& b) {
    double s = std::clamp(sev01, 0.0, 1.0);
    // simple gradient: 0 → grün (0,255,0), 1 → rot (255,0,0)
    r = static_cast<uint8_t>(255.0 * s);
    g = static_cast<uint8_t>(255.0 * (1.0 - s));
    b = 0;
}

void RealHardware::UpdateLedsFromSeverity(const std::vector<double>& sev) {
    if (!leds_.isInitialized()) return;

    // Map jede Severity auf eine LED-Farbe
    for (int i = 0; i < std::min<int>(leds_.count(), static_cast<int>(sev.size())); ++i) {
        uint8_t r, g, b;
        SeverityToRGB(sev[i], r, g, b);
        leds_.setPixel(i, r, g, b);
    }
    std::string err;
    leds_.show(&err); // Fehler bei Bedarf loggen
}

// ----- Konstruktor / Destruktor -----
RealHardware::RealHardware(const AppConfigView& cfg, int led_pin, int led_channel, int led_count)
: cfg_(cfg),
  // RelayController: Pins aus AppConfig 
  relays_({ .chip_path="/dev/gpiochip0",
            .pins = {14, 15, 24, 23}, // ACHTUNG: 18 freigehalten für WS281x <<-- HARDWARE ÄNDERN
            .active_high = true,
            .initial_on = false,
            .consumer   = "sosesta-relay" }),
  redlab_{},  // TODO: ggf. Konstruktor-Parameter
  tca_{},     // TODO: ggf. Bus/Adresse
  ina_{},     // TODO: Shunt/MaxCurrent konfigurieren, wenn nötig
  leds_({     // LEDStrip::Config
        .gpio_pin    = led_pin,
        .led_count   = led_count,
        .dma_channel = 10,
        .invert      = 0,
        .brightness  = 64,
        .strip_type  = WS2811_STRIP_GRB,
        .freq        = WS2811_TARGET_FREQ,
        .channel_index = led_channel })
{
    sensors_.resize(kNumChannels);
    for (int i = 0; i < kNumChannels; ++i) {
        sensors_[i].channel = i;
    }

    // --- Init Reihenfolge ---
    // 1) Relais (sicherer Grundzustand)
    {
        std::string err;
        if (!relays_.init(&err)) {
            // TODO: Log/Fehlerbehandlung
        } else {
            relays_.setAll(false, nullptr);
        }
    }

    // 2) TCA9548A, INA219
    tca_.Init(); // TODO: Rückgabewert prüfen/loggen
    ina_.Init(); // TODO: Rückgabewert prüfen/kalibrieren (shunt, max current)

    // 3) RedLab DAQ verbinden
    redlab_.Connect(); // TODO: Rückgabewert prüfen/loggen

    // 4) LED-Strip initialisieren
    {
        std::string err;
        if (!leds_.init(&err)) {
            // TODO: Log/Fehlerbehandlung
        } else {
            leds_.clear();
            leds_.show();
        }
    }

    // Relais-Adapter anlegen (nur falls benötigt)
    g_relays_adapter.emplace(relays_);
}

RealHardware::~RealHardware() {
    // LEDs aus
    std::string err;
    if (leds_.isInitialized()) {
        leds_.clear(&err);
        leds_.show(&err);
    }

    // DAQ trennen
    redlab_.Disconnect();

    // Relais freigeben
    relays_.shutdown();

    // LED-Strip finalisieren
    leds_.shutdown();
}

// ----- IRelays-Zugriff -----
IRelays& RealHardware::Relays() {
    // Wenn dein RelayController bereits IRelays erbt/implementiert:
    // return relays_;
    // Sonst Adapter:
    return *g_relays_adapter;
}

// ----- Sensor-Update -----
void RealHardware::UpdateSensors() {
    std::lock_guard<std::mutex> lock(mtx_);

    std::vector<double> sev(kNumChannels, 0.0);

    for (int ch = 0; ch < kNumChannels; ++ch) {
        SensorData& s = sensors_[ch];

        // --- TCA: Kanal selektieren ---
        tca_.Select(ch); // TODO: Fehler prüfen

        // --- INA219 lesen ---
        if (auto v = ina_.Read()) {
            // erwartetes Format: tuple(bus_V, current_mA, power_mW)
            s.bus_V      = std::get<0>(*v);
            s.current_mA = std::get<1>(*v);
            s.power_mW   = std::get<2>(*v);
        }

        // --- RedLab lesen ---
        if (auto v = redlab_.Read(ch)) {
            s.redlab_V = *v;
        }

        // --- Präsenzheuristik (wie von dir skizziert) ---
        const bool voltage_ok = !(s.redlab_V >= 1.30 && s.redlab_V <= 1.60);
        const bool current_ok = s.current_mA > 0.3;
        s.present = voltage_ok || current_ok;

        // --- Statusflags anhand Konfig ---
        const bool supply_ok   = InRange(s.bus_V,        cfg_.supply_voltage_threshold);
        const bool signal_ok   = InRange(s.redlab_V,     cfg_.redlab_neg_threshold)
                              || InRange(s.redlab_V,     cfg_.redlab_pos_threshold);
        const bool current_ok2 = InRange(s.current_mA,   cfg_.presence_current_threshold);

        // --- Fehlerzähler: steigern bei ok->nicht ok ---
        if (s.supply_ok  && !supply_ok)   ++s.supply_err_cnt;
        if (s.signal_ok  && !signal_ok)   ++s.signal_err_cnt;
        if (s.current_ok && !current_ok2) ++s.current_err_cnt;

        s.supply_ok  = supply_ok;
        s.signal_ok  = signal_ok;
        s.current_ok = current_ok2;

        // --- Severity 0..1 für LED ---
        double sev_val = 0.0;
        if (!s.supply_ok)  sev_val += 0.5;
        if (!s.signal_ok)  sev_val += 0.3;
        if (!s.current_ok) sev_val += 0.2;
        sev[ch] = std::clamp(sev_val, 0.0, 1.0);
    }

    // --- LEDs aktualisieren ---
    UpdateLedsFromSeverity(sev);
}
