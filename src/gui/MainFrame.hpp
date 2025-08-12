#pragma once
#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/timer.h>
#include <wx/dataview.h>
#include "App.hpp"
#include "ChannelWidget.hpp"

class MainFrame : public wxFrame {
public:
    explicit MainFrame(SoSeStaApp* app);

private:
    SoSeStaApp* app_;

    // "Aktuelle Konfiguration" über zwei Zeilen
    wxStaticText* cfg_text_line1_ = nullptr;
    wxStaticText* cfg_text_line2_ = nullptr;

    wxButton* edit_btn_ = nullptr;
    wxButton* font_btn_ = nullptr; // Schriftgröße…

    wxPanel* channels_panel_ = nullptr;
    std::array<ChannelWidget*,8> channels{};

    // Ereignis-Log (tabellarisch)
    wxDataViewListCtrl* error_view_ = nullptr;
    wxButton *btn_toggle_ = nullptr, *btn_start_ = nullptr, *btn_stop_ = nullptr, *btn_archive_ = nullptr;
    wxButton *btn_err_export_ = nullptr, *btn_err_clear_ = nullptr;
    wxStaticText* timer_label_ = nullptr;

    bool test_running_ = false;
    bool relay_state_ = false;
    std::chrono::system_clock::time_point start_ts_;
    int test_duration_sec_;

    wxTimer ui_timer_;
    wxTimer toggle_timer_;

    // Zustands-Tracking (Kipp-Punkte)
    std::array<bool,8> prev_supply_ok_{};
    std::array<bool,8> prev_signal_ok_{};
    std::array<bool,8> prev_current_ok_{};
    bool prev_init_ = false;

    // Aufbau
    void BuildConfigDisplay(wxWindow* parent);
    void BuildChannels(wxWindow* parent);
    void BuildErrors(wxWindow* parent);
    void BuildControls(wxWindow* parent);
    void RefreshConfigLabel();

    // Handlers
    void OnToggle(wxCommandEvent&);
    void OnStart(wxCommandEvent&);
    void OnStop(wxCommandEvent&);
    void OnArchive(wxCommandEvent&);
    void OnUiTick(wxTimerEvent&);
    void OnToggleTick(wxTimerEvent&);
    void OnChangeFont(wxCommandEvent&);
    void OpenConfigEditor();

    // Helpers
    void ChangeFontSize(int ptSize);
    static void SetFontSizeRecursive(wxWindow* win, int ptSize);

    // Updates
    void UpdateChannels();
    void UpdateErrors();
    void UpdateTimer();

    // Logging
    void LogEvent(const wxString& when, int ch, const wxString& sn,
                  const wxString& kind, const wxString& detail,
                  const wxString& relay, const wxString& sev);
    void ExportErrorsCSV();

    wxDECLARE_EVENT_TABLE();
};
