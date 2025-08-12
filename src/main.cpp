#include "app/App.hpp"
#include "gui/MainFrame.hpp"
#include "hw/HardwareFactory.hpp"

wxIMPLEMENT_APP(SoSeStaApp);

bool SoSeStaApp::OnInit() {
    if (!wxApp::OnInit()) return false;

    // App-Config (Defaults; später ggf. laden)
    AppConfigView cfg;

    // Hardware über Factory (Mock/Real per -DSOSESTA_USE_MOCK)
    hw = MakeHardware(cfg).release();

    auto* frame = new MainFrame(this);
    frame->Show();
    return true;
}

int SoSeStaApp::OnExit() {
    delete hw; hw = nullptr;
    return wxApp::OnExit();
}
