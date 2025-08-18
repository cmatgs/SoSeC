#pragma once
#include <optional>
#include <string>
#include <tuple>
#include <cstdint>

// regisin/ina219 C++-Header
#include <ina219.h>

/**
 * @brief C++-Adapter für regisin/ina219.
 *
 * Liefert Einzelmessungen als (bus_V, current_mA, power_mW).
 * Konfiguration über Shunt-Widerstand und erwarteten Max-Strom,
 * sowie INA219-Betriebsparameter (Range/Gain/ADC).
 */
class INA219Manager {
public:
    struct Config {
        // Elektrische Parameter
        float shunt_ohms        = 0.1f;   ///< Shunt-Widerstand [Ohm]
        float max_expected_amps = 3.2f;   ///< Erwarteter Max-Strom [A] (für Kalibrierung)

        // I²C-Parameter (sofern von der verwendeten Lib unterstützt; default: /dev/i2c-1, 0x40)
        int      i2c_bus        = 1;      ///< Standard RPi: Bus 1 (/dev/i2c-1) – ggf. ignoriert, wenn Lib es intern fixed.
        uint8_t  i2c_address    = 0x40;   ///< Standard-Addr des INA219 – nur nutzen, wenn Lib SetAddress() o.ä. bietet.

        // Mess-/Filter-Parameter
        RANGE    range          = RANGE_16V;      ///< Bus-Range: 16V/32V (je nach Lib-Enum)
        GAIN     gain           = GAIN_8_320MV;   ///< Shunt-ADC Gain
        ADC      bus_adc        = ADC_12BIT;      ///< Bus-ADC Auflösung/Avg
        ADC      shunt_adc      = ADC_12BIT;      ///< Shunt-ADC Auflösung/Avg
    };

    explicit INA219Manager(const Config& cfg = {});
    ~INA219Manager();

    // Non-copyable, movable
    INA219Manager(const INA219Manager&) = delete;
    INA219Manager& operator=(const INA219Manager&) = delete;
    INA219Manager(INA219Manager&&) noexcept;
    INA219Manager& operator=(INA219Manager&&) noexcept;

    /**
     * @brief Initialisiert den Sensor (I²C + Kalibrierung).
     * @return true bei Erfolg, sonst false (Fehlertext optional in err)
     */
    bool Init(std::string* err = nullptr);

    /**
     * @brief Liest eine Messung.
     * @return (bus_V, current_mA, power_mW) oder std::nullopt bei Fehler.
     */
    std::optional<std::tuple<double,double,double>> Read(std::string* err = nullptr) const;

    /**
     * @brief Optionales Freigeben von Ressourcen (die regisin-Lib braucht i.d.R. nichts).
     */
    void Shutdown();

    bool isInitialized() const { return initialized_; }
    const Config& cfg() const { return cfg_; }

private:
    static void setErr(std::string* err, const std::string& msg);

    Config cfg_;
    INA219  ina_;            ///< regisin-Treiberinstanz
    bool    initialized_ = false;
};
