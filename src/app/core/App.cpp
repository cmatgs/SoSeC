#include "app/core/App.hpp"
#include "gui/MainFrame.hpp"
#include "hw/HardwareFactory.hpp"

wxIMPLEMENT_APP(App);

bool App::OnInit() {
    if (!wxApp::OnInit())
        return false;

    LoadConfig();

    hardware = MakeHardware(MakeConfigView(config_software), config_hardware);

    main_frame_ = new MainFrame(nullptr, config_software);
    
    main_frame_->AttachHardware(hardware);
    main_frame_->Show(true);
    return true;
}

int App::OnExit() {
    SaveConfig();
    if (hardware) {
        hardware->Shutdown();
        hardware.reset();
    }
    return wxApp::OnExit();
}

void App::LoadConfig() {
    // Defaultwerte verwenden
}

void App::SaveConfig() {
}

void App::StartTest() {
    state.test_running     = true;
    state.elapsed_seconds  = 0;
    state.error_count_total= 0;
}

void App::StopTest() {
    state.test_running = false;
}
