#pragma once
#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/timer.h>
#include <wx/dataview.h>
#include <array>
#include <vector>
#include <string>
#include <chrono>
#include <functional>

#include "config/ConfigSoftware.hpp"
#include "services/LoggerService.hpp"
#include "services/CsvExporter.hpp"
#include "services/TestRunner.hpp"
#include "app/data/SensorData.hpp"
#include "gui/ChannelWidget.hpp"
#include "hw/IHardware.hpp"

class MainFrame : public wxFrame {
public:
    explicit MainFrame(wxWindow* parent, ConfigSoftware& cfg);

    // aktuelle HW (Mock oder Real) ankoppeln
    void AttachHardware(const std::shared_ptr<sosesta::hw::IHardware>& hw);

private:
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
    void LogEvent(
        const wxString& when, 
        int ch, 
        const wxString& sn,
        const wxString& kind, 
        const wxString& detail,
        const wxString& relay, 
        const wxString& sev);
    void ExportErrorsCSV();

private:
    // Konfiguration + Dienste (aktuelle Architektur)
    ConfigSoftware& cfg_;
    LoggerService   logger_;
    CsvExporter     exporter_;
    TestRunner      test_runner_;     // liefert Sensors() und Step()

    // "Aktuelle Konfiguration" über zwei Zeilen
    wxStaticText* cfg_text_line1_ = nullptr;

    wxButton* edit_btn_ = nullptr;
    wxButton* font_btn_ = nullptr; // Schriftgröße…

    wxPanel* channels_panel_ = nullptr;

    // ChannelWidget ist bei dir ein Control mit Signatur:
    // ChannelWidget(wxWindow*, int, std::function<bool(int)>, std::vector<std::string>&)
    std::array<ChannelWidget*,8> channels{};

    // Ereignis-Log (tabellarisch)
    wxDataViewListCtrl* error_view_ = nullptr;
    wxButton *btn_toggle_ = nullptr, *btn_start_ = nullptr, *btn_stop_ = nullptr, *btn_archive_ = nullptr;
    wxButton *btn_err_export_ = nullptr, *btn_err_clear_ = nullptr;
    wxStaticText* timer_label_ = nullptr;

    // Test-/UI-Status
    bool test_running_ = false;
    bool relay_state_  = false; // Gesamtzustand (wir schalten alle Relais gemeinsam)
    std::chrono::system_clock::time_point start_ts_{};
    int test_duration_sec_ = 0;

    // Timer (IDs wie in deiner Vorlage)
    wxTimer ui_timer_;
    wxTimer toggle_timer_;

    // Zustands-Tracking (Kipp-Punkte)
    std::array<bool,8> prev_supply_ok_{};
    std::array<bool,8> prev_signal_ok_{};
    std::array<bool,8> prev_current_ok_{};
    bool prev_init_ = false;

    // einfache SN-Liste, falls ChannelWidget eine Anzeige erwartet
    std::array<std::string,8> serial_numbers_{};

    // Relay-Paarlabel für ChannelWidget (falls genutzt)
    std::vector<std::string> relay_labels_ { "K0/1", "K2/3", "K4/5", "K6/7" };

    // Callback für optionales Paar-Schalten aus ChannelWidget (hier Dummy)
    std::function<bool(int)> on_toggle_pair_ =
        [this](int /*pairIdx*/){ /* optional: einzelnes Paar schalten */ return false; };

    wxDECLARE_EVENT_TABLE();
};
