#pragma once
#include <memory>
#include <vector>

struct ConfigSoftware;        // forward
class LoggerService;          // forward
struct SensorData;            // forward

namespace sosesta { namespace hw { struct IHardware; } }

class TestRunner {
public:
    explicit TestRunner(ConfigSoftware& cfg, LoggerService& log);

    void SetHardware(const std::shared_ptr<sosesta::hw::IHardware>& hw);

    void Start();
    void Stop();
    void Step();
    void ToggleRelays();
    const std::vector<SensorData>& Sensors() const { return sensors_; }


private:
    void EnsureSensorsSize();

    ConfigSoftware& cfg_;
    LoggerService&  log_;

    std::weak_ptr<sosesta::hw::IHardware> hw_;  // kein Besitz
    bool running_   = false;
    bool relays_on_ = false;

    std::vector<SensorData> sensors_;
    static constexpr int kNumChannels = 8;
};
