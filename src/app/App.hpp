// src/app/App.hpp
#pragma once
#include <memory>
#include <array>
#include <string>
#include <wx/wx.h>
#include "hw/IHardware.hpp"

struct AppConfig {
    // GUI erwartet diese Felder (siehe MainFrame.cpp)
    int    test_duration_sec = 60*60;      // 1 h
    int    update_interval_ms = 500;       // GUI-Refresh

    double redlab_pos_threshold[2] = { 2.0, 5.0 };
    double redlab_neg_threshold[2] = {-5.0,-2.0 };
    double supply_voltage_threshold[2] = {4.7, 5.5};
    double presence_current_threshold[2] = { 0.5, 25.0 }; // mA

    int    toggle_interval_sec = 600; // 10 min
    int    test_interval_sec   = 5;   // Sekunden (GUI nutzt das aktuell als Toggle-Timer)
};

// View für Hardware (read-only)
inline AppConfigView MakeConfigView(const AppConfig& c) {
    AppConfigView v;
    v.redlab_pos_threshold[0] = c.redlab_pos_threshold[0];
    v.redlab_pos_threshold[1] = c.redlab_pos_threshold[1];
    v.redlab_neg_threshold[0] = c.redlab_neg_threshold[0];
    v.redlab_neg_threshold[1] = c.redlab_neg_threshold[1];
    v.redlab_idle_voltage_range[0] = 1.3;
    v.redlab_idle_voltage_range[1] = 1.6;
    v.supply_voltage_threshold[0] = c.supply_voltage_threshold[0];
    v.supply_voltage_threshold[1] = c.supply_voltage_threshold[1];
    v.max_current_mA = c.presence_current_threshold[1];
    v.toggle_interval_sec = c.toggle_interval_sec;
    v.test_interval_sec   = c.test_interval_sec;
    return v;
}

class SoSeStaApp : public wxApp {
public:
    AppConfig config;
    std::unique_ptr<IHardware> hw;
    std::array<std::string, 8> serial_numbers{}; // für MainFrame::UpdateErrors()

    virtual bool OnInit() override;
};
