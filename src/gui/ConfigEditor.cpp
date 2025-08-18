#include "gui/ConfigEditor.hpp"
#include <wx/valnum.h>

ConfigEditorDlg::ConfigEditorDlg(wxWindow* parent, ConfigSoftware& cfg)
: wxDialog(parent, wxID_ANY, wxString::FromUTF8("Schwellen bearbeiten"),
           wxDefaultPosition, wxSize(460, 420)) // etwas höher für das neue Feld
, cfg_(cfg)
{
    auto* root = new wxBoxSizer(wxVERTICAL);

    const double hours = cfg_.test_duration_sec / 3600.0;

    t_hours_      = new wxTextCtrl(this, wxID_ANY, wxString::Format("%.2f", hours));
    t_pos_lo_     = new wxTextCtrl(this, wxID_ANY, wxString::Format("%.2f", cfg_.redlab_pos_threshold[0]));
    t_pos_hi_     = new wxTextCtrl(this, wxID_ANY, wxString::Format("%.2f", cfg_.redlab_pos_threshold[1]));
    t_neg_lo_     = new wxTextCtrl(this, wxID_ANY, wxString::Format("%.2f", cfg_.redlab_neg_threshold[0]));
    t_neg_hi_     = new wxTextCtrl(this, wxID_ANY, wxString::Format("%.2f", cfg_.redlab_neg_threshold[1]));
    t_supply_lo_  = new wxTextCtrl(this, wxID_ANY, wxString::Format("%.2f", cfg_.supply_voltage_threshold[0]));
    t_supply_hi_  = new wxTextCtrl(this, wxID_ANY, wxString::Format("%.2f", cfg_.supply_voltage_threshold[1]));
    t_curr_min_   = new wxTextCtrl(this, wxID_ANY, wxString::Format("%.2f", cfg_.presence_current_threshold[0]));
    t_curr_max_   = new wxTextCtrl(this, wxID_ANY, wxString::Format("%.2f", cfg_.presence_current_threshold[1]));
    // NEU: Feld für I_max
    t_imax_       = new wxTextCtrl(this, wxID_ANY, wxString::Format("%.2f", cfg_.max_current_mA));

    auto addRow = [&](const char* label, wxTextCtrl* ctrl){
        auto* row = new wxBoxSizer(wxHORIZONTAL);
        row->Add(new wxStaticText(this, wxID_ANY, wxString::FromUTF8(label)), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 6);
        row->Add(ctrl, 1);
        root->Add(row, 0, wxEXPAND|wxALL, 4);
    };

    addRow("Dauer [h]",            t_hours_);
    addRow("Pos-Lo [V]",           t_pos_lo_);
    addRow("Pos-Hi [V]",           t_pos_hi_);
    addRow("Neg-Lo [V]",           t_neg_lo_);
    addRow("Neg-Hi [V]",           t_neg_hi_);
    addRow("Supply-Lo [V]",        t_supply_lo_);
    addRow("Supply-Hi [V]",        t_supply_hi_);
    addRow("Präsenzstrom min [mA]", t_curr_min_);
    addRow("Präsenzstrom max [mA]", t_curr_max_);
    // NEU:
    addRow("I_max [mA]",            t_imax_);

    auto* buttons = new wxBoxSizer(wxHORIZONTAL);
    auto* ok     = new wxButton(this, wxID_OK, wxString::FromUTF8("Speichern"));
    auto* cancel = new wxButton(this, wxID_CANCEL, wxString::FromUTF8("Abbrechen"));
    buttons->Add(ok, 0, wxRIGHT, 6);
    buttons->Add(cancel, 0);

    root->Add(buttons, 0, wxALIGN_RIGHT|wxALL, 6);
    SetSizerAndFit(root);

    ok->Bind(wxEVT_BUTTON, &ConfigEditorDlg::OnSave, this);
}

void ConfigEditorDlg::OnSave(wxCommandEvent&) {
    auto parse = [&](wxTextCtrl* t, double& out){
        double v = 0.0; 
        t->GetValue().ToDouble(&v); 
        out = v;
    };

    double hours = 0.0;
    t_hours_->GetValue().ToDouble(&hours);
    cfg_.test_duration_sec = static_cast<int>(hours * 3600.0);

    parse(t_pos_lo_,     cfg_.redlab_pos_threshold[0]);
    parse(t_pos_hi_,     cfg_.redlab_pos_threshold[1]);
    parse(t_neg_lo_,     cfg_.redlab_neg_threshold[0]);
    parse(t_neg_hi_,     cfg_.redlab_neg_threshold[1]);
    parse(t_supply_lo_,  cfg_.supply_voltage_threshold[0]);
    parse(t_supply_hi_,  cfg_.supply_voltage_threshold[1]);
    parse(t_curr_min_,   cfg_.presence_current_threshold[0]);
    parse(t_curr_max_,   cfg_.presence_current_threshold[1]);
    // NEU:
    parse(t_imax_,       cfg_.max_current_mA);

    EndModal(wxID_OK);
}
