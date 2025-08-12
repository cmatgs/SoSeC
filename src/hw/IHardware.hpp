#pragma once
#include <string>
#include <vector>

// 8 Kanäle gesamt
constexpr int kNumChannels = 8;

struct SensorData {
    int channel = 0;

    // Messwerte
    double bus_V      = 0.0;   // INA219 Busspannung
    double current_mA = 0.0;   // INA219 Strom
    double power_mW   = 0.0;   // INA219 Leistung
    double redlab_V   = 0.0;   // RedLab Spannung

    // Meta
    bool present   = false;
    std::string serial_number;

    // Status
    bool supply_ok  = false;
    bool signal_ok  = false;
    bool current_ok = false;

    // Zähler
    int supply_err_cnt  = 0;
    int signal_err_cnt  = 0;
    int current_err_cnt = 0;
};

struct IRelays {
    virtual ~IRelays() = default;
    virtual void ToggleAll() = 0;
    virtual bool GetState(int idx) const = 0; // 0..3
};

struct IHardware {
    virtual ~IHardware() = default;
    virtual void UpdateSensors() = 0;
    virtual const std::vector<SensorData>& Sensors() const = 0;
    virtual IRelays& Relays() = 0;
};

// Minimaler Blick auf AppConfig – wir lesen nur Grenzwerte, die du schon nutzt
struct AppConfigView {
    double redlab_pos_threshold[2]       = { 2.0,  5.0 };   ///< Positivbereich RedLab-Signal [V]
    double redlab_neg_threshold[2]       = {-5.0, -2.0 };   ///< Negativbereich RedLab-Signal [V]

    // Bereich der Leerlaufspannung am RedLab, 1.3–1.6 V
    double redlab_idle_voltage_range[2]  = { 1.3, 1.6 };    ///< V

    double supply_voltage_threshold[2]   = { 4.7,  5.5 };   ///< Versorgungsspannung [V]
    double max_current_mA                = 25.0;            ///< mA

    int toggle_interval_sec              = 600;             ///< 10 Minuten in Sekunden
    int test_interval_sec                = 5;               ///< Standard-Testintervall [s]
};