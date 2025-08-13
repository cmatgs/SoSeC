#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include "hw/IHardware.hpp"

class MockRelays : public IRelays {
public:
    void ToggleAll() override {
        for (auto& s : state_) s = !s;
    }
    bool GetState(int idx) const override {
        return (idx >= 0 && idx < 4) ? state_[idx] : false;
    }
private:
    bool state_[4] = {false, false, false, false};
};

class MockHardware : public IHardware {
public:
    struct Options {
        unsigned seed        = 42;
        int      num_channels= kNumChannels;  // 8
        double   bus_sigma_V = 0.05;          // Rauschen
        double   current_sigma_mA = 0.8;
        double   redlab_sigma_V   = 3.0;
        bool     inject_faults    = true;
    };

    // *** WICHTIG: by-value Default + DEFINIERTER Konstruktor mit {}
    explicit MockHardware(const AppConfigView& cfg, Options opt = Options{})
    : cfg_(cfg),
      sensors_(opt.num_channels > 0 ? opt.num_channels : kNumChannels),
      rng_(opt.seed ? opt.seed : std::random_device{}()),
      n_(0.0, 1.0),
      opt_(opt)
    {
        if (sensors_.size() > static_cast<std::size_t>(kNumChannels))
            sensors_.resize(kNumChannels);
        for (std::size_t i = 0; i < sensors_.size(); ++i) {
            sensors_[i].channel = static_cast<int>(i);
        }
    }

    void UpdateSensors() override {
        // einfache Simulation mit Normalverteilungen
        const float bus_mid       = 5.0f;
        const float current_mid   = 5.0f;    // mA
        const float redlab_posmid = 3.0f;

        for (auto& s : sensors_) {
            s.present     = true;
            s.bus_V       = bus_mid       + static_cast<float>(opt_.bus_sigma_V      * n_(rng_));
            s.current_mA  = std::max(0.0f, current_mid   + static_cast<float>(opt_.current_sigma_mA * n_(rng_)));
            s.power_mW    = s.bus_V * s.current_mA;
            s.redlab_V    = redlab_posmid + static_cast<float>(opt_.redlab_sigma_V   * n_(rng_));

            // Bewertung vs. cfg_
            s.supply_ok = (s.bus_V >= cfg_.supply_voltage_threshold[0] &&
                           s.bus_V <= cfg_.supply_voltage_threshold[1]);

            const bool in_pos = (s.redlab_V >= cfg_.redlab_pos_threshold[0] &&
                                 s.redlab_V <= cfg_.redlab_pos_threshold[1]);
            const bool in_neg = (s.redlab_V >= cfg_.redlab_neg_threshold[0] &&
                                 s.redlab_V <= cfg_.redlab_neg_threshold[1]);
            s.signal_ok = in_pos || in_neg;

            s.current_ok = (s.current_mA >= 0.0f &&
                            s.current_mA <= static_cast<float>(cfg_.max_current_mA));

            // Fehlerzähler inkrementieren
            s.supply_error_counter  += (!s.supply_ok);
            s.signal_error_counter  += (!s.signal_ok);
            s.current_error_counter += (!s.current_ok);

            // optional: sporadische Fehler
            if (opt_.inject_faults) {
                const int r = static_cast<int>(rng_() % 120);
                if (r == 0) s.signal_ok = false;
            }
        }
    }

    const std::vector<SensorData>& Sensors() const override { return sensors_; }
    IRelays& Relays() override { return relays_; }

private:
    AppConfigView                       cfg_;
    std::vector<SensorData>             sensors_;
    MockRelays                          relays_;
    std::mt19937                        rng_;
    std::normal_distribution<double>    n_;
    Options                             opt_;
};
