#pragma once
#include <wx/wx.h>
#include "App.hpp"

class ConfigEditorDlg : public wxDialog {
public:
    ConfigEditorDlg(wxWindow* parent, Config& cfg);
private:
    Config& cfg_;
    wxTextCtrl *test_duration_h_, *pos_thr_, *neg_thr_, *pres_thr_, *supply_thr_;

    void OnSave(wxCommandEvent&);
    std::string Arr2Str(const double a[2]);
    bool ParsePair(const wxString& s, double out[2], wxString* err);
};
