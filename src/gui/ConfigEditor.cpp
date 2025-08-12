#include "ConfigEditor.hpp"
#include <sstream>

ConfigEditorDlg::ConfigEditorDlg(wxWindow* parent, Config& cfg)
: wxDialog(parent, wxID_ANY, "Konfiguration bearbeiten", wxDefaultPosition, wxSize(520,320), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
, cfg_(cfg)
{
    auto* root = new wxBoxSizer(wxVERTICAL);
    auto row = [&](const wxString& label, wxTextCtrl*& ctrl, const wxString& val){
        auto* hz = new wxBoxSizer(wxHORIZONTAL);
        hz->Add(new wxStaticText(this, wxID_ANY, label), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 8);
        ctrl = new wxTextCtrl(this, wxID_ANY, val);
        hz->Add(ctrl, 1, wxEXPAND);
        root->Add(hz, 0, wxEXPAND|wxALL, 6);
    };

    double hours = cfg_.test_duration_sec / 3600.0;
    row("Testdauer [h]", test_duration_h_, wxString::Format("%.1f", hours));
    row("PosSchwelle [V, V]", pos_thr_, Arr2Str(cfg_.redlab_pos_threshold));
    row("NegSchwelle [V, V]", neg_thr_, Arr2Str(cfg_.redlab_neg_threshold));
    row("Präsenzstrom [mA, mA]", pres_thr_, Arr2Str(cfg_.presence_current_threshold));
    row("Versorgungsspannung [V, V]", supply_thr_, Arr2Str(cfg_.supply_voltage_threshold));

    auto* btns = CreateSeparatedButtonSizer(wxOK|wxCANCEL);
    root->Add(btns, 0, wxEXPAND|wxALL, 6);

    Bind(wxEVT_BUTTON, &ConfigEditorDlg::OnSave, this, wxID_OK);

    SetSizerAndFit(root);
    CentreOnParent();
}

std::string ConfigEditorDlg::Arr2Str(const double a[2]){
    std::ostringstream os; os<<a[0]<<", "<<a[1]; return os.str();
}

bool ConfigEditorDlg::ParsePair(const wxString& s, double out[2], wxString* err){
    double a,b; char c;
    std::string str = s.ToStdString();
    std::istringstream is(str);
    if(!(is>>a)) { if(err) *err="Zahl erwartet"; return false; }
    if(!(is>>c)) { if(err) *err="Komma/Trenner fehlt"; return false; }
    if(!(is>>b)) { if(err) *err="zweite Zahl erwartet"; return false; }
    out[0]=a; out[1]=b; return true;
}

void ConfigEditorDlg::OnSave(wxCommandEvent&){
    // Validierung & Übernahme
    double hours;
    if(!test_duration_h_->GetValue().ToDouble(&hours) || hours<=0){
        wxMessageBox("Ungültige Testdauer.", "Fehler", wxICON_ERROR); return;
    }
    cfg_.test_duration_sec = static_cast<int>(hours*3600.0);

    auto try_pair = [&](wxTextCtrl* ctrl, double out[2], const char* what){
        wxString err;
        if(!ParsePair(ctrl->GetValue(), out, &err)){
            wxMessageBox(wxString::Format("%s: %s", what, err), "Fehler", wxICON_ERROR);
            throw 1;
        }
    };

    try {
        try_pair(pos_thr_, cfg_.redlab_pos_threshold, "PosSchwelle");
        try_pair(neg_thr_, cfg_.redlab_neg_threshold, "NegSchwelle");
        try_pair(pres_thr_, cfg_.presence_current_threshold, "Präsenzstrom");
        try_pair(supply_thr_, cfg_.supply_voltage_threshold, "Versorgung");
    } catch(...) { return; }

    EndModal(wxID_OK);
}
