#pragma once
#include <wx/wx.h>
#include <array>
#include <string>
#include <vector>
#include <chrono>

struct Config {
    int    test_duration_sec   = 2 * 60 * 60;           // 2h
    int    update_interval_ms  = 300;                   // GUI Refresh
    int    test_interval_sec   = 5;                     // Relais-Toggle-Intervall
    double redlab_pos_threshold[2] = {3.0, 3.3};
    double redlab_neg_threshold[2] = {-0.1, 0.1};
    double presence_current_threshold[2] = {1.0, 5.0};  // mA
    double supply_voltage_threshold[2] = {4.75, 5.25};  // V
    // frei für weitere Werte (current_threshold etc.)
};

struct SensorData {
    int channel = 0;
    bool present = true;
    bool supply_ok = true;
    bool signal_ok = true;
    bool current_ok = true;
    double current_mA = 0.0;
    double bus_V = 0.0;
    double redlab_V = 0.0;

    int supply_err_cnt = 0;
    int signal_err_cnt = 0;
    int current_err_cnt = 0;

    std::string serial_number;
};

struct RelaysIface {
    virtual ~RelaysIface() = default;
    virtual void ToggleAll() = 0;
    virtual bool GetState(int relay_idx) const = 0; // 0..3
};

struct HardwareIface {
    virtual ~HardwareIface() = default;
    virtual void UpdateSensors() = 0;
    virtual const std::vector<SensorData>& Sensors() const = 0; // 8 Kanäle
    virtual RelaysIface& Relays() = 0;
};

class SoSeStaApp : public wxApp {
public:
    Config config;
    std::array<std::string,8> serial_numbers{};
    HardwareIface* hw = nullptr; // gesetzt in OnInit

    bool OnInit() override;
    int OnExit() override;
};
wxDECLARE_APP(SoSeStaApp);
