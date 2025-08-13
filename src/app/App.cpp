// src/app/App.cpp
#include "app/App.hpp"
#include "gui/MainFrame.hpp"
#include "hw/HardwareFactory.hpp"

wxIMPLEMENT_APP(SoSeStaApp);

bool SoSeStaApp::OnInit() {
    auto view = MakeConfigView(config);
    hw = MakeHardware(view);  // besitzt die Hardware

    auto* f = new MainFrame(this);
    f->Show(true);
    return true;
}
