#pragma once
#include <cstdint>

struct SensorData {
    int   channel = 0;

    // Messwerte
    float bus_V        = 0.0f;   // Versorgungsspannung [V]
    float current_mA   = 0.0f;   // Strom [mA]
    float power_mW     = 0.0f;   // Leistung [mW]
    float redlab_V     = 0.0f;   // RedLab-Signal [V]

    // Zustände
    bool  present      = false;
    bool  supply_ok    = false;
    bool  signal_ok    = false;
    bool  current_ok   = false;

    // Fehlerzähler
    int   supply_error_counter  = 0;
    int   signal_error_counter  = 0;
    int   current_error_counter = 0;
};
