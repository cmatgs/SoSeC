#pragma once
#include <memory>
#include <vector>

struct ConfigSoftware;        // Konfiguration der Software
class LoggerService;          // Protokollierungsdienst
struct SensorData;            // Sensordaten

namespace sosesta { namespace hw { struct IHardware; } }  // Hardware-Interface

class TestRunner {
public:
    explicit TestRunner(ConfigSoftware& cfg, LoggerService& log);

    void SetHardware(const std::shared_ptr<sosesta::hw::IHardware>& hw);    // Setzt die Hardware (Mock oder Real)

    void Start();
    void Stop();
    void Step();
    void ToggleRelays();
    const std::vector<SensorData>& Sensors() const { return sensors_; }

private:
    void EnsureSensorsSize();   // Stellt sicher, dass der Sensorvektor die richtige Größe hat

    ConfigSoftware& cfg_;
    LoggerService&  log_;

    std::weak_ptr<sosesta::hw::IHardware> hw_; // Nicht-besitzend, da IHardware nicht kopierbar 
    bool running_   = false;
    bool relays_on_ = false;

    std::vector<SensorData> sensors_;
    static constexpr int kNumChannels = 8;
};
