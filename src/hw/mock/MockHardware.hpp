#pragma once
#include "hw/IHardware.hpp"   // bringt: SensorData, IRelays, IHardware, AppConfigView, kNumChannels
#include <random>
#include <vector>
#include <cstddef>

/**
 * Schlanker Relais‑Mock, kompatibel zu IRelays.
 * - 4 Relais (Index 0..3)
 * - ToggleAll() invertiert alle
 * - GetState(i) liest Zustand (außerhalb des Bereichs -> false)
 */
class MockRelays : public IRelays {
    bool state_[4] = {false,false,false,false};
public:
    void ToggleAll() override {
        for (bool &s : state_) s = !s;
    }
    bool GetState(int idx) const override {
        if (idx < 0 || idx > 3) return false;
        return state_[idx];
    }

    // Zusatz (nur für Tests): gezielt setzen
    void _Set(int idx, bool on) {
        if (idx >= 0 && idx < 4) state_[idx] = on;
    }
};

/**
 * MockHardware
 * - liefert realistisch schwankende Messwerte
 * - Statusflags + Fehlerzähler gemäß NEUER AppConfigView:
 *   * redlab_idle_voltage_range  (1.3..1.6 V) -> Präsenz-Heuristik
 *   * supply_voltage_threshold   -> supply_ok
 *   * redlab_pos/neg_threshold   -> signal_ok
 *   * max_current_mA             -> current_ok (0 < I <= max)
 */
class MockHardware : public IHardware {
public:
    struct Options {
        std::size_t num_channels = kNumChannels; // 1..8
        bool inject_faults = false;              // sporadische Ausfälle/Spikes simulieren
        unsigned seed = 0;                       // 0 => zufällig per random_device
        // Rauschstärken (ungefähr)
        double bus_sigma_V      = 0.05;          // ±50 mV
        double current_sigma_mA = 0.5;           // ±0.5 mA
        double redlab_sigma_V   = 1.2;           // ±1.2 V
    };

    explicit MockHardware(const AppConfigView& cfg, Options opt = {});
    : cfg_(cfg),
      sensors_(opt.num_channels ? opt.num_channels : kNumChannels),
      rng_(opt.seed ? opt.seed : std::random_device{}()),
      n_{0.0, 1.0},
      opt_(opt)
    {
        if (sensors_.size() > kNumChannels) sensors_.resize(kNumChannels);
        for (std::size_t i = 0; i < sensors_.size(); ++i) {
            sensors_[i].channel = static_cast<int>(i);
            sensors_[i].present = true;
            sensors_[i].serial_number = "MOCK-" + std::to_string(i+1);
        }
    }

    // --- IHardware ---
    void UpdateSensors() override {
        // Mittelwerte: an deine Ranges angepasst
        const double bus_mid       = mid(cfg_.supply_voltage_threshold);
        const double current_mid   = 0.5 * cfg_.max_current_mA; // „typischer“ Strom ~ halber Max-Wert
        const double redlab_posmid = mid(cfg_.redlab_pos_threshold);

        for (auto& s : sensors_) {
            // Grundrauschen um die Mittelwerte
            s.bus_V       = bus_mid       + opt_.bus_sigma_V      * n_(rng_);
            s.current_mA  = current_mid   + opt_.current_sigma_mA * n_(rng_);
            s.redlab_V    = redlab_posmid + opt_.redlab_sigma_V   * n_(rng_);
            if (s.current_mA < 0.0) s.current_mA = 0.0; // Strom negativ vermeiden
            s.power_mW    = s.bus_V * s.current_mA;     // grobe Näherung (mW~V*mA)

            // Optional: Fehler-Injektion (selten)
            if (opt_.inject_faults) {
                const int r = static_cast<int>(rng_() % 120); // ~1/120 Chance je Sample
                if (r == 0) { s.present = false; s.current_mA = 0.0; s.power_mW = 0.0; }
                else if (r == 1) { s.redlab_V += 5.0; }
                else if (r == 2) { s.bus_V = cfg_.supply_voltage_threshold[0] - 0.3; }
            } else {
                s.present = true;
            }

            // Präsenz-Heuristik:
            // "Leerlaufbereich" 1.3..1.6 V gilt als nicht präsent
            const bool redlab_not_idle = !in_range(s.redlab_V, cfg_.redlab_idle_voltage_range);
            const bool current_present = s.current_mA > 0.3; // Heuristik; anpassbar
            s.present = s.present && (redlab_not_idle || current_present);

            // Status nach AppConfigView
            const bool supply_ok  = in_range(s.bus_V, cfg_.supply_voltage_threshold);
            const bool signal_ok  = in_range(s.redlab_V, cfg_.redlab_neg_threshold)
                                 || in_range(s.redlab_V, cfg_.redlab_pos_threshold);
            const bool current_ok = (s.current_mA > 0.0) && (s.current_mA <= cfg_.max_current_mA);

            // Fehlerzähler erhöhen, wenn nicht ok
            if (!supply_ok)  ++s.supply_err_cnt;
            if (!signal_ok)  ++s.signal_err_cnt;
            if (!current_ok) ++s.current_err_cnt;

            s.supply_ok  = supply_ok;
            s.signal_ok  = signal_ok;
            s.current_ok = current_ok;
        }
    }

    const std::vector<SensorData>& Sensors() const override { return sensors_; }
    IRelays& Relays() override { return relays_; }

private:
    // Hilfen
    static inline bool in_range(double v, const double rng[2]) {
        return v >= rng[0] && v <= rng[1];
    }
    static inline double mid(const double rng[2]) {
        return 0.5 * (rng[0] + rng[1]);
    }

    // Daten
    AppConfigView cfg_;
    std::vector<SensorData> sensors_;
    MockRelays relays_;

    // Zufall
    std::mt19937 rng_;
    std::normal_distribution<double> n_;
    Options opt_;
};
