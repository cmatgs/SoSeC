#include "App.hpp"
#include "MainFrame.hpp"
#include "MockBackend.hpp"

wxIMPLEMENT_APP(SoSeStaApp);

bool SoSeStaApp::OnInit(){
    if (!wxApp::OnInit()) return false;
    // Mock-Hardware; später durch echte Implementierung ersetzen
    hw = new MockHardware();
    auto* frame = new MainFrame(this);
    frame->Show();
    return true;
}
int SoSeStaApp::OnExit(){
    delete hw; hw=nullptr;
    return wxApp::OnExit();
}
