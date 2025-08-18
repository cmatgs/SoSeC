#include "hw/mock/MockHardware.hpp"
#include <algorithm>
#include <chrono>
#include <cassert>

namespace sosesta { namespace hw {

MockHardware::MockHardware(const ConfigSoftwareView& cfg, MockOptions opt)
    : cfg_(cfg)
    , opt_(opt)
    , rng_(opt.seed)
    , relay_state_(static_cast<size_t>(opt_.num_relays), false) // alle Relais AUS
{}

void MockHardware::Initialize() {
    initialized_ = true;
}

void MockHardware::Shutdown() {
    initialized_ = false;
}

void MockHardware::UpdateSensors(std::vector<SensorData>& sensors) {
    if (!initialized_) return;

    sensors.resize(static_cast<size_t>(opt_.num_channels));

    // Hilfswerte aus Konfig:
    const double bus_mid   = 0.5 * (cfg_.supply_voltage_threshold[0] + cfg_.supply_voltage_threshold[1]);
    const double cur_max   = cfg_.max_current_mA;
    const double red_mid_p = 0.5 * (cfg_.redlab_pos_threshold[0] + cfg_.redlab_pos_threshold[1]);
    const double red_mid_n = 0.5 * (cfg_.redlab_neg_threshold[0] + cfg_.redlab_neg_threshold[1]);

    for (int ch = 0; ch < opt_.num_channels; ++ch) {
        auto& s = sensors[static_cast<size_t>(ch)];
        s.channel = ch; // wichtig für GUI (Relaiszuordnung ch/2)

        // Relaispaar bestimmen (0..num_relays-1)
        const int  relay_idx = ch / 2;
        const bool on        = relay_state_[static_cast<size_t>(relay_idx)];

        // Busspannung
        const double bus_noise = n_bus_(rng_) * opt_.bus_sigma_V;
        s.bus_V = std::max(0.0, bus_mid + bus_noise);

        // Strom (einfaches Modell)
        const double cur_noise = n_cur_(rng_) * opt_.current_sigma_mA;
        s.current_mA = std::clamp((on ? 0.6 * cur_max : 0.05 * cur_max) + cur_noise, 0.0, cur_max);

        // RedLab (umschalten zwischen +/− Bereich)
        const double red_noise = n_red_(rng_) * opt_.redlab_sigma_V;
        s.redlab_V = (on ? red_mid_p : red_mid_n) + red_noise;

        // Status
        s.present   = s.current_mA >= (0.2 * cur_max);
        s.supply_ok = (s.bus_V   >= cfg_.supply_voltage_threshold[0] &&
                       s.bus_V   <= cfg_.supply_voltage_threshold[1]);
        s.signal_ok = (s.redlab_V >= cfg_.redlab_neg_threshold[0] &&
                       s.redlab_V <= cfg_.redlab_pos_threshold[1]);

        // Fehlerzähler (simple Logik)
        if (!s.supply_ok) ++s.supply_error_counter;
        if (!s.signal_ok) ++s.signal_error_counter;
        if (s.current_mA < cfg_.presence_current_threshold[0] ||
            s.current_mA > cfg_.presence_current_threshold[1]) {
            ++s.current_error_counter;
        }

        // Zeitstempel
        s.timestamp_ms = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
    }
}

void MockHardware::ToggleRelay(int channel_pair, bool state) {
    if (channel_pair < 0 || channel_pair >= opt_.num_relays) return;
    relay_state_[static_cast<size_t>(channel_pair)] = state;
}

void MockHardware::TurnAllRelaysOn() {
    std::fill(relay_state_.begin(), relay_state_.end(), true);
}

void MockHardware::TurnAllRelaysOff() {
    std::fill(relay_state_.begin(), relay_state_.end(), false);
}

}} // namespace sosesta::hw
