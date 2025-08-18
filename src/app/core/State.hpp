#pragma once
#include <array>
#include "app/data/SensorData.hpp"

struct State {
    bool test_running     = false;
    int  elapsed_seconds  = 0;
    int  error_count_total= 0;

    // letzter Sensorschnappschuss (z. B. für Anzeige/Export)
    std::array<SensorData, kNumChannels> last_snapshot{};
};
