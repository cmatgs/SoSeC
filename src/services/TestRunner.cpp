#include "services/TestRunner.hpp"
#include <algorithm>

#include "config/ConfigSoftware.hpp"
#include "services/LoggerService.hpp"
#include "app/data/SensorData.hpp"
#include "hw/IHardware.hpp"

using sosesta::hw::IHardware;

TestRunner::TestRunner(ConfigSoftware& cfg, LoggerService& log)
    : cfg_(cfg)
    , log_(log)
{
    EnsureSensorsSize();
}

void TestRunner::SetHardware(const std::shared_ptr<IHardware>& hw) {
    hw_ = hw; // nicht-besitzend via weak_ptr
}

void TestRunner::EnsureSensorsSize() {
    sensors_.resize(static_cast<size_t>(kNumChannels));
}

void TestRunner::Start() {
    auto hw = hw_.lock();
    if (!hw) { running_ = true; relays_on_ = false; return; }
    hw->Initialize();
    running_   = true;
    relays_on_ = false;
}

void TestRunner::Stop() {
    if (auto hw = hw_.lock()) { hw->Shutdown(); }
    running_ = false;
}

void TestRunner::Step() {
    if (!running_) return;
    EnsureSensorsSize();

    if (auto hw = hw_.lock()) {
        hw->UpdateSensors(sensors_);
    } else {
        std::fill(sensors_.begin(), sensors_.end(), SensorData{});
    }
}

void TestRunner::ToggleRelays() {
    if (auto hw = hw_.lock()) {
        if (relays_on_) { hw->TurnAllRelaysOff(); relays_on_ = false; }
        else            { hw->TurnAllRelaysOn();  relays_on_ = true;  }
    }
}
