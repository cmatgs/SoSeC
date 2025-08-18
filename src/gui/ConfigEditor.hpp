#pragma once
#include <wx/wx.h>
#include "config/ConfigSoftware.hpp"

class ConfigEditorDlg : public wxDialog {
public:
    ConfigEditorDlg(wxWindow* parent, ConfigSoftware& cfg);

private:
    void OnSave(wxCommandEvent&);

    ConfigSoftware& cfg_;

    wxTextCtrl* t_hours_    = nullptr;
    wxTextCtrl* t_pos_lo_   = nullptr;
    wxTextCtrl* t_pos_hi_   = nullptr;
    wxTextCtrl* t_neg_lo_   = nullptr;
    wxTextCtrl* t_neg_hi_   = nullptr;
    wxTextCtrl* t_supply_lo_= nullptr;
    wxTextCtrl* t_supply_hi_= nullptr;
    wxTextCtrl* t_curr_min_ = nullptr;
    wxTextCtrl* t_curr_max_ = nullptr;

    // NEU:
    wxTextCtrl* t_imax_     = nullptr;
};
