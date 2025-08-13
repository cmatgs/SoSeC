#pragma once
#include <wx/wx.h>

// Vorwärtsdeklaration, damit kein Zyklus entsteht:
struct AppConfig;

class ConfigEditorDlg : public wxDialog {
public:
    ConfigEditorDlg(wxWindow* parent, AppConfig& cfg);

private:
    AppConfig& cfg_;
    wxTextCtrl* t_hours_ = nullptr;
    wxTextCtrl* t_pos_lo_= nullptr;
    wxTextCtrl* t_pos_hi_= nullptr;
    wxTextCtrl* t_neg_lo_= nullptr;
    wxTextCtrl* t_neg_hi_= nullptr;
    wxTextCtrl* t_supply_lo_= nullptr;
    wxTextCtrl* t_supply_hi_= nullptr;

    void OnSave(wxCommandEvent&);
};
