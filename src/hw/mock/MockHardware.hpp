#pragma once
#include <vector>
#include <random>

#include "hw/IHardware.hpp"
#include "config/ConfigSoftware.hpp"
#include "app/data/SensorData.hpp"

namespace sosesta { namespace hw {

// Freier Options-Typ (nicht mehr verschachtelt), damit Default-Argument {} problemlos funktioniert.
struct MockOptions {
    unsigned seed             = 42;
    int      num_channels     = kNumChannels; // aus SensorData.hpp (aktuell 8)
    int      num_relays       = 4;            // Paare 0/1, 2/3, 4/5, 6/7
    double   bus_sigma_V      = 0.05;
    double   current_sigma_mA = 0.8;
    double   redlab_sigma_V   = 3.0;
    bool     inject_faults    = true;
};

struct MockHardware : IHardware {
    explicit MockHardware(const ConfigSoftwareView& cfg,
                          MockOptions opt = {});   // <— Default-Argument funktioniert jetzt sauber

    // IHardware
    void Initialize() override;
    void Shutdown() override;
    void UpdateSensors(std::vector<SensorData>& sensors) override;
    void ToggleRelay(int channel_pair, bool state) override; // 0..(num_relays-1)
    void TurnAllRelaysOn() override;
    void TurnAllRelaysOff() override;

private:
    ConfigSoftwareView cfg_;
    MockOptions        opt_;
    bool               initialized_ = false;

    std::mt19937 rng_;
    std::normal_distribution<double> n_bus_{0.0, 1.0};
    std::normal_distribution<double> n_cur_{0.0, 1.0};
    std::normal_distribution<double> n_red_{0.0, 1.0};

    // 4 Relais (für 8 Kanäle als Paare)
    std::vector<bool> relay_state_;
};

}} // namespace sosesta::hw
