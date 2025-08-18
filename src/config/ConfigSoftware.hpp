#pragma once
#include <array>

// Laufzeit-/Prüfparameter, änderbar via GUI
struct ConfigSoftware {
    int update_interval_ms = 200;   // GUI-Update
    int test_interval_sec  = 10;    // Prüfintervall
    int test_duration_sec  = 3600;  // Gesamtdauer

    // Schwellen
    std::array<double,2> redlab_pos_threshold       {  2.0,  5.0 };
    std::array<double,2> redlab_neg_threshold       { -5.0, -2.0 };
    std::array<double,2> supply_voltage_threshold   {  4.5,  5.5 };
    std::array<double,2> presence_current_threshold {  5.0, 100.0 };

    // Für das Mock-Präsenzmodell
    double max_current_mA = 25.0;
};

// „View“ für Hardware/Mock (nur lesbar benötigte Felder)
struct ConfigSoftwareView {
    int update_interval_ms = 0;
    int test_interval_sec  = 0;
    int test_duration_sec  = 0;

    std::array<double,2> redlab_pos_threshold{};
    std::array<double,2> redlab_neg_threshold{};
    std::array<double,2> supply_voltage_threshold{};

    // Optional: Mock kann auch Präsenzgrenzen kennen
    std::array<double,2> presence_current_threshold{};
    double max_current_mA = 0.0;
};

inline ConfigSoftwareView MakeConfigView(const ConfigSoftware& c) {
    ConfigSoftwareView v;
    v.update_interval_ms         = c.update_interval_ms;
    v.test_interval_sec          = c.test_interval_sec;   
    v.test_duration_sec          = c.test_duration_sec;
    v.redlab_pos_threshold       = c.redlab_pos_threshold;
    v.redlab_neg_threshold       = c.redlab_neg_threshold;
    v.supply_voltage_threshold   = c.supply_voltage_threshold;
    v.presence_current_threshold = c.presence_current_threshold;
    v.max_current_mA             = c.max_current_mA;
    return v;
}
