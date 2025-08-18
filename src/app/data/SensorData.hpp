#pragma once
#include <cstdint>

inline constexpr int kNumChannels = 8;

struct SensorData {
    int    channel = 0;        // 0..7

    // Messwerte
    double bus_V      = 0.0;   // Versorgungsspannung
    double current_mA = 0.0;   // Stromaufnahme
    double power_mW   = 0.0;   // optional
    double redlab_V   = 0.0;   // RedLab-Signal

    // Status
    bool present    = false;
    bool supply_ok  = false;
    bool signal_ok  = false;

    // Fehlerzähler
    int supply_error_counter  = 0;
    int signal_error_counter  = 0;
    int current_error_counter = 0;

    // Zeit
    std::uint64_t timestamp_ms = 0;
};
