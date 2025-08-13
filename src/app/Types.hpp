#pragma once
#include <cstdint>

struct SensorData {
    int   channel = 0;
    float bus_voltage = 0.0f;   // V
    float current     = 0.0f;   // mA
    float power       = 0.0f;   // mW
    float redlab_signal = 0.0f; // V
    bool  present     = false;
    bool  supply_ok   = false;
    int   supply_error_counter = 0;
    bool  signal_ok   = false;
    int   signal_error_counter = 0;
};
