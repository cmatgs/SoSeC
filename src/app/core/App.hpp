#pragma once
#include <wx/wx.h>
#include <memory>

#include "config/ConfigHardware.hpp"
#include "config/ConfigSoftware.hpp"
#include "app/core/State.hpp"   

namespace sosesta { namespace hw { struct IHardware; } }

class App : public wxApp {
public:
    bool OnInit() override;
    int  OnExit() override;

    void LoadConfig();
    void SaveConfig();

    void StartTest();
    void StopTest();

private:
    ConfigHardware config_hardware;
    ConfigSoftware config_software;

    State state;

    std::shared_ptr<sosesta::hw::IHardware> hardware;
    class MainFrame* main_frame_ = nullptr;
};
