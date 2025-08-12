#pragma once
#include "IHardware.hpp"
#include "RelayController.hpp"
#include "RedLabDAQ.hpp"
#include "TCA9548A.hpp"
#include "INA219.hpp"
#include "LEDStrip.hpp"
#include <mutex>
#include <optional>

/**
 * @brief Reale Hardware-Implementierung (Produktivbetrieb).
 *
 * Kapselt:
 *  - TCA9548A I2C-Multiplexer (Kanaalselektion)
 *  - INA219 (Strom/Spannung/Leistung)
 *  - RedLab DAQ (Analogsignal)
 *  - RelayController (GPIO-Relais via libgpiod)
 *  - LEDStrip (WS281x, Statusvisualisierung)
 *
 * Thread-safe: UpdateSensors() sperrt intern einen Mutex.
 */
class RealHardware : public IHardware {
public:
    /**
     * @param cfg           schreibgeschützter Config-View (Grenzwerte etc.)
     * @param led_pin       GPIO für WS281x (Default 18 empfohlen). Achtung: 10 (MOSI) nur mit SPI/PCM-Betriebsart!
     * @param led_channel   WS281x-Channel (0/1)
     * @param led_count     Anzahl LEDs am Strip
     */
    RealHardware(const AppConfigView& cfg, int led_pin = 18, int led_channel = 0, int led_count = 8);
    ~RealHardware() override;

    void UpdateSensors() override;
    const std::vector<SensorData>& Sensors() const override { return sensors_; }

    // Falls IHardware::IRelays die Schnittstelle ist: Adapter bereitstellen.
    // Option A: Wenn RelayController bereits IRelays implementiert, dann einfach "return relays_;"
    // Option B: Kleiner Adapter (siehe unten in der .cpp).
    IRelays& Relays() override;

private:
    // --- Konfiguration ---
    AppConfigView cfg_;

    // --- HW-Komponenten ---
    RelayController relays_;
    RedLabDAQ       redlab_;
    TCA9548A        tca_;
    INA219Manager   ina_;
    LEDStrip        leds_;

    // --- Daten & Schutz ---
    std::vector<SensorData> sensors_;
    mutable std::mutex mtx_;

    // --- Hilfen ---
    static inline bool InRange(double v, const double range[2]) {
        return v >= range[0] && v <= range[1];
    }

    // LED-Hilfen
    static void SeverityToRGB(double sev01, uint8_t& r, uint8_t& g, uint8_t& b);
    void UpdateLedsFromSeverity(const std::vector<double>& sev);
};
