#include "App.hpp"
#include "gui/MainFrame.hpp"

bool SoSeStaApp::OnInit() {
    auto* f = new MainFrame(this);
    f->Show();
    return true;
}
