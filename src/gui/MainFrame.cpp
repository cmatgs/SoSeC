#include "gui/MainFrame.hpp"
#include "gui/ConfigEditor.hpp"

#include <wx/numdlg.h>
#include <wx/sizer.h>
#include <wx/filefn.h>
#include <wx/statline.h>
#include <wx/ffile.h>
#include <wx/datetime.h>
#include <algorithm>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_TIMER(1000, MainFrame::OnUiTick)
    EVT_TIMER(1001, MainFrame::OnToggleTick)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(wxWindow* parent, ConfigSoftware& cfg)
: wxFrame(parent, wxID_ANY, wxString::FromUTF8("SoSeSta – Prüfstation (wx)"),
          wxDefaultPosition, wxSize(1280,800))
, cfg_(cfg)
, logger_()
, exporter_(logger_)
, test_runner_(cfg_, logger_)
, ui_timer_(this, 1000)
, toggle_timer_(this, 1001)
{
    auto* root = new wxBoxSizer(wxVERTICAL);

    auto* cfgp = new wxPanel(this);
    BuildConfigDisplay(cfgp);
    root->Add(cfgp, 0, wxEXPAND|wxALL, 6);

    channels_panel_ = new wxPanel(this);
    BuildChannels(channels_panel_);
    root->Add(channels_panel_, 1, wxEXPAND|wxLEFT|wxRIGHT, 6);

    auto* ctrl = new wxPanel(this);
    BuildControls(ctrl);
    root->Add(ctrl, 0, wxEXPAND|wxALL, 6);

    auto* err = new wxPanel(this);
    BuildErrors(err);
    root->Add(err, 2, wxEXPAND|wxALL, 6);

    SetSizer(root);
    CentreOnScreen();

    test_duration_sec_ = cfg_.test_duration_sec;
    ui_timer_.Start(std::max(50, cfg_.update_interval_ms));
}

void MainFrame::AttachHardware(const std::shared_ptr<sosesta::hw::IHardware>& hw) {
    test_runner_.SetHardware(hw);
    test_runner_.Start();
}

void MainFrame::BuildConfigDisplay(wxWindow* parent) {
    auto* s = new wxBoxSizer(wxHORIZONTAL);
    auto* box = new wxStaticBoxSizer(wxVERTICAL, parent, wxString::FromUTF8("Aktuelle Konfiguration"));

    // eine Zeile Text
    cfg_text_line1_ = new wxStaticText(parent, wxID_ANY, "");
    RefreshConfigLabel();

    // Buttons rechts
    auto* btnRow = new wxBoxSizer(wxHORIZONTAL);
    edit_btn_ = new wxButton(parent, wxID_ANY, wxString::FromUTF8("⚙️ Schwellen bearbeiten"));
    edit_btn_->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){ OpenConfigEditor(); });

    font_btn_ = new wxButton(parent, wxID_ANY, wxString::FromUTF8("🔤 Schriftgröße…"));
    font_btn_->Bind(wxEVT_BUTTON, &MainFrame::OnChangeFont, this);

    btnRow->Add(edit_btn_, 0, wxRIGHT, 8);
    btnRow->Add(font_btn_, 0);

    // links (nur 1 Zeile), rechts (Buttons)
    auto* row = new wxBoxSizer(wxHORIZONTAL);
    row->Add(cfg_text_line1_, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    row->Add(btnRow, 0, wxALIGN_CENTER_VERTICAL);

    box->Add(row, 0, wxEXPAND | wxALL, 8);
    s->Add(box, 1, wxEXPAND | wxALL, 0);
    parent->SetSizerAndFit(s);

    // Wrap bei Größenänderung
    parent->Bind(wxEVT_SIZE, [this](wxSizeEvent& e){
        e.Skip();
        if (!cfg_text_line1_) return;
        int total = cfg_text_line1_->GetParent()->GetClientSize().GetWidth();
        int wrapW = std::max(200, total - 260); // Platz für Buttons berücksichtigen
        cfg_text_line1_->Wrap(wrapW);
    });
}


void MainFrame::RefreshConfigLabel(){
    const auto& c = cfg_;
    if (cfg_text_line1_)
        cfg_text_line1_->SetLabel(
            wxString::FromUTF8("Dauer: ") << wxString::Format("%.1f h    ", c.test_duration_sec / 3600.0)
            << wxString::FromUTF8("Maximalstrom: ") << wxString::Format("%.1f mA    ", c.max_current_mA)
            << wxString::FromUTF8("PosSchwelle: ") << wxString::Format("[%.2f, %.2f] V    ", c.redlab_pos_threshold[0], c.redlab_pos_threshold[1])
            << wxString::FromUTF8("NegSchwelle: ") << wxString::Format("[%.2f, %.2f] V    ", c.redlab_neg_threshold[0], c.redlab_neg_threshold[1])
            << wxString::FromUTF8("Präsenzspannung: ") << wxString::Format("[%.2f, %.2f] mA    ", c.presence_current_threshold[0], c.presence_current_threshold[1])
            << wxString::FromUTF8("Versorgung: ") << wxString::Format("[%.2f, %.2f] V", c.supply_voltage_threshold[0], c.supply_voltage_threshold[1])
        );
}



// ───────────────────────────────────────────
// Schriftgröße ändern (rekursiv)
void MainFrame::OnChangeFont(wxCommandEvent&) {
    int currentPt = 10;
    if (cfg_text_line1_) currentPt = std::max(6, cfg_text_line1_->GetFont().GetPointSize());

    wxNumberEntryDialog dlg(this,
        wxString::FromUTF8("Neue Schriftgröße in Punkt (6–32):"),
        wxString::FromUTF8("Schriftgröße"),
        wxString::FromUTF8("Schriftgröße ändern"),
        currentPt, 6, 32);

    if (dlg.ShowModal() == wxID_OK) {
        ChangeFontSize(dlg.GetValue());
    }
}

void MainFrame::ChangeFontSize(int ptSize) {
    SetFontSizeRecursive(this, ptSize);
    Layout();
    Refresh();
}

void MainFrame::SetFontSizeRecursive(wxWindow* win, int ptSize) {
    if (!win) return;
    wxFont f = win->GetFont();
    if (f.IsOk()) {
        f.SetPointSize(ptSize);
        win->SetFont(f);
    }
    for (wxWindowList::compatibility_iterator node = win->GetChildren().GetFirst();
         node; node = node->GetNext())
    {
        wxWindow* child = node->GetData();
        SetFontSizeRecursive(child, ptSize);
    }
}

void MainFrame::BuildChannels(wxWindow* parent){
    auto* grid = new wxGridSizer(1, 8, 6, 6);
    for (int i = 0; i < 8; ++i) {
        auto* pane  = new wxPanel(parent);
        auto* sizer = new ChannelWidget(pane, i, on_toggle_pair_, serial_numbers_);
        channels[i] = sizer;
        pane->SetSizer(sizer);
        grid->Add(pane, 1, wxEXPAND);
    }
    parent->SetSizer(grid);
}

void MainFrame::BuildErrors(wxWindow* parent){
    auto* root = new wxBoxSizer(wxVERTICAL);
    auto* box  = new wxStaticBoxSizer(wxVERTICAL, parent, wxString::FromUTF8("Ereignis-Log (Fehlerübersicht)"));

    // Toolbar
    auto* toolbar = new wxBoxSizer(wxHORIZONTAL);
    btn_err_export_ = new wxButton(parent, wxID_ANY, "CSV exportieren");
    btn_err_clear_  = new wxButton(parent, wxID_ANY, "Leeren");
    toolbar->Add(btn_err_export_, 0, wxRIGHT, 6);
    toolbar->Add(btn_err_clear_,  0, wxRIGHT, 6);
    toolbar->AddStretchSpacer();

    // Tabelle (inkl. SN)
    error_view_ = new wxDataViewListCtrl(parent, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxDV_ROW_LINES|wxDV_VERT_RULES|wxDV_MULTIPLE);
    error_view_->AppendTextColumn("Zeit",      wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT,  wxDATAVIEW_COL_SORTABLE);
    error_view_->AppendTextColumn("Kanal",     wxDATAVIEW_CELL_INERT, 60,  wxALIGN_RIGHT, wxDATAVIEW_COL_SORTABLE);
    error_view_->AppendTextColumn("SN",        wxDATAVIEW_CELL_INERT, 110, wxALIGN_LEFT,  wxDATAVIEW_COL_SORTABLE);
    error_view_->AppendTextColumn("Art",       wxDATAVIEW_CELL_INERT, 110, wxALIGN_LEFT,  wxDATAVIEW_COL_SORTABLE);
    error_view_->AppendTextColumn("Detail",    wxDATAVIEW_CELL_INERT, 320, wxALIGN_LEFT,  wxDATAVIEW_COL_RESIZABLE);
    error_view_->AppendTextColumn("Relais",    wxDATAVIEW_CELL_INERT, 70,  wxALIGN_CENTER,wxDATAVIEW_COL_SORTABLE);
    error_view_->AppendTextColumn("Severity",  wxDATAVIEW_CELL_INERT, 90,  wxALIGN_LEFT,  wxDATAVIEW_COL_SORTABLE);

    // Events
    btn_err_export_->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){ ExportErrorsCSV(); });
    btn_err_clear_->Bind(wxEVT_BUTTON,  [this](wxCommandEvent&){ error_view_->DeleteAllItems(); });

    box->Add(toolbar, 0, wxEXPAND|wxALL, 4);
    box->Add(error_view_, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
    root->Add(box, 1, wxEXPAND|wxALL, 0);
    parent->SetSizer(root);
}

void MainFrame::BuildControls(wxWindow* parent){
    auto* s = new wxBoxSizer(wxHORIZONTAL);
    btn_toggle_  = new wxButton(parent, wxID_ANY, wxString::FromUTF8("🔁 Relais toggeln"));
    btn_start_   = new wxButton(parent, wxID_ANY, wxString::FromUTF8("▶️ Start Test"));
    btn_stop_    = new wxButton(parent, wxID_ANY, wxString::FromUTF8("⏹ Stop Test"));
    btn_archive_ = new wxButton(parent, wxID_ANY, wxString::FromUTF8("📂 Archiv öffnen"));
    timer_label_ = new wxStaticText(parent, wxID_ANY, "00:00:00");

    btn_stop_->Enable(false);

    btn_toggle_->Bind(wxEVT_BUTTON, &MainFrame::OnToggle, this);
    btn_start_->Bind(wxEVT_BUTTON, &MainFrame::OnStart, this);
    btn_stop_->Bind(wxEVT_BUTTON, &MainFrame::OnStop, this);
    btn_archive_->Bind(wxEVT_BUTTON, &MainFrame::OnArchive, this);

    s->Add(btn_toggle_, 0, wxRIGHT, 6);
    s->Add(btn_start_, 0, wxRIGHT, 6);
    s->Add(btn_stop_, 0, wxRIGHT, 6);
    s->Add(btn_archive_, 0, wxRIGHT, 6);
    s->AddStretchSpacer();
    s->Add(timer_label_, 0, wxALIGN_CENTER_VERTICAL);

    parent->SetSizer(s);
}

void MainFrame::OnToggle(wxCommandEvent&){
    if (wxMessageBox(wxString::FromUTF8("Relais manuell toggeln? Nur bei Bedarf."),
                     "Sicherheitsabfrage", wxYES_NO|wxICON_WARNING) != wxYES)
        return;

    // aktuelle Architektur: alle Relais umlegen
    test_runner_.ToggleRelays();
    relay_state_ = !relay_state_;
    for(auto* w : channels) if (w) w->SetRelayState(relay_state_);
}

void MainFrame::OnStart(wxCommandEvent&){
    test_running_ = true;
    start_ts_ = std::chrono::system_clock::now();
    test_duration_sec_ = cfg_.test_duration_sec;

    btn_start_->Enable(false);
    btn_stop_->Enable(true);
    btn_toggle_->Enable(false);
    btn_archive_->Enable(false);
    for(auto* w : channels) if (w) w->DisableSerialInput();

    // Start: Relais einmal einschalten
    if (!relay_state_) {
        test_runner_.ToggleRelays();
        relay_state_ = true;
        for(auto* w : channels) if (w) w->SetRelayState(true);
    }

    toggle_timer_.Start(cfg_.test_interval_sec * 1000);
}

void MainFrame::OnStop(wxCommandEvent&){
    test_running_ = false;
    btn_start_->Enable(true);
    btn_stop_->Enable(false);
    btn_toggle_->Enable(true);
    btn_archive_->Enable(true);
    toggle_timer_.Stop();
    // TODO: CSV/Excel finalisieren, Log speichern
}

void MainFrame::OnArchive(wxCommandEvent&){
    wxString path = wxGetCwd();
#ifdef __WXMSW__
    ::wxExecute("explorer \"" + path + "\"");
#elif defined(__WXOSX__)
    ::wxExecute("open \"" + path + "\"");
#else
    ::wxExecute("xdg-open \"" + path + "\"");
#endif
}

void MainFrame::OnUiTick(wxTimerEvent&){
    // neue Sensorwerte holen
    test_runner_.Step();
    UpdateChannels();
    UpdateErrors();
    UpdateTimer();
}

void MainFrame::OnToggleTick(wxTimerEvent&){
    if (!test_running_) return;
    test_runner_.ToggleRelays();
    relay_state_ = !relay_state_;
    for(auto* w : channels) if (w) w->SetRelayState(relay_state_);
}

void MainFrame::UpdateChannels(){
    const auto& vec = test_runner_.Sensors();
    const size_t n = std::min(vec.size(), channels.size());
    for(size_t i=0;i<n;++i){
        if (channels[i]) channels[i]->UpdateFrom(vec[i]);
    }
}

// Änderungen (OK↔Fehler) erkennen und protokollieren
void MainFrame::UpdateErrors(){
    const auto& S = test_runner_.Sensors();
    if (S.empty()) return;

    // Schwellwerte aus Config
    const auto& c = cfg_;

    if (!prev_init_){
        for (size_t i=0;i<S.size() && i<8;++i){
            prev_supply_ok_[i] = S[i].supply_ok;
            prev_signal_ok_[i] = S[i].signal_ok;
            // current_ok aus Stromfenster abgeleitet
            const bool cur_ok = (S[i].current_mA >= c.presence_current_threshold[0] &&
                                 S[i].current_mA <= c.presence_current_threshold[1]);
            prev_current_ok_[i]= cur_ok;
        }
        prev_init_ = true;
        return;
    }

    const wxString now = wxDateTime::Now().FormatISOCombined(' ');
    for (size_t i=0;i<S.size() && i<8;++i){
        const auto& s = S[i];

        wxString sn = serial_numbers_[i].empty()
                      ? wxString("-")
                      : wxString::FromUTF8(serial_numbers_[i].c_str());
        const wxString relay = relay_state_ ? "ON" : "OFF";

        // Versorgung
        if (s.supply_ok != prev_supply_ok_[i]){
            if (!s.supply_ok){
                LogEvent(now, int(i), sn, "Versorgung",
                         wxString::Format("V=%.2f außerhalb [%.2f, %.2f]",
                             s.bus_V, c.supply_voltage_threshold[0], c.supply_voltage_threshold[1]),
                         relay, "ERROR");
            } else {
                LogEvent(now, int(i), sn, "Versorgung", "wieder OK", relay, "OK");
            }
            prev_supply_ok_[i] = s.supply_ok;
        }

        // Signal (RedLab)
        if (s.signal_ok != prev_signal_ok_[i]){
            if (!s.signal_ok){
                LogEvent(now, int(i), sn, "Signal",
                         wxString::Format("RedLab=%.2f außerhalb [%g,%g] / [%g,%g]",
                            s.redlab_V,
                            c.redlab_neg_threshold[0], c.redlab_neg_threshold[1],
                            c.redlab_pos_threshold[0], c.redlab_pos_threshold[1]),
                         relay, "ERROR");
            } else {
                LogEvent(now, int(i), sn, "Signal", "wieder OK", relay, "OK");
            }
            prev_signal_ok_[i] = s.signal_ok;
        }

        // Strom → current_ok ableiten
        const bool current_ok_now = (s.current_mA >= c.presence_current_threshold[0] &&
                                     s.current_mA <= c.presence_current_threshold[1]);
        if (current_ok_now != prev_current_ok_[i]){
            if (!current_ok_now){
                LogEvent(now, int(i), sn, "Strom",
                         wxString::Format("I=%.2f mA jenseits [%.2f, %.2f] mA",
                             s.current_mA, c.presence_current_threshold[0], c.presence_current_threshold[1]),
                         relay, "ERROR");
            } else {
                LogEvent(now, int(i), sn, "Strom", "wieder OK", relay, "OK");
            }
            prev_current_ok_[i] = current_ok_now;
        }

        // Präsenz (optional Hinweis)
        if (!s.present){
            LogEvent(now, int(i), sn, "Sensor Erkannt", "Sensor nicht erkannt", relay, "WARN");
        }
    }
}

void MainFrame::UpdateTimer(){
    if (!test_running_){
        timer_label_->SetLabel("00:00:00");
        return;
    }
    auto now = std::chrono::system_clock::now();
    auto end = start_ts_ + std::chrono::seconds(test_duration_sec_);
    if (now >= end){
        timer_label_->SetLabel("00:00:00");
        wxCommandEvent evt;
        OnStop(evt);
        return;
    }
    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(end-now).count();
    int h = static_cast<int>(remaining / 3600);
    int m = static_cast<int>((remaining % 3600) / 60);
    int s = static_cast<int>(remaining % 60);
    timer_label_->SetLabel(wxString::Format("%02d:%02d:%02d", h,m,s));
}

void MainFrame::LogEvent(const wxString& when, int ch, const wxString& sn,
                         const wxString& kind, const wxString& detail,
                         const wxString& relay, const wxString& sev)
{
    wxVector<wxVariant> row;
    row.push_back(wxVariant(when));                         // Zeit
    row.push_back(wxVariant(wxString::Format("%d", ch+1))); // Kanal (1..8)
    row.push_back(wxVariant(sn));                           // SN
    row.push_back(wxVariant(kind));                         // Art
    row.push_back(wxVariant(detail));                       // Detail
    row.push_back(wxVariant(relay));                        // Relais
    row.push_back(wxVariant(sev));                          // Severity
    error_view_->AppendItem(row);
}

void MainFrame::ExportErrorsCSV(){
    wxFileDialog dlg(this, wxString::FromUTF8("CSV exportieren"), "", "ereignis_log.csv",
        "CSV Dateien (*.csv)|*.csv", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal()!=wxID_OK) return;

    // Binär öffnen + BOM (Excel-freundlich)
    wxFFile f(dlg.GetPath(), "wb");
    if (!f.IsOpened()){
        wxMessageBox(wxString::FromUTF8("Datei konnte nicht geschrieben werden."), "Fehler", wxICON_ERROR);
        return;
    }
    static const unsigned char bom[3] = {0xEF,0xBB,0xBF};
    f.Write(bom, 3);

    auto W = [&](const wxString& s){ f.Write(s.ToUTF8()); };

    W("Zeit,Kanal,SN,Art,Detail,Relais,Severity\n");
    const unsigned n = error_view_->GetItemCount();
    for (unsigned i=0;i<n;++i){
        auto get = [&](int col){ wxVariant v; error_view_->GetValue(v,i,col); return v.GetString(); };
        wxString line = wxString::Format("\"%s\",%s,\"%s\",\"%s\",%s,%s,%s\n",
            get(0), get(1), get(2), get(3), get(4), get(5), get(6));
        W(line);
    }
}

void MainFrame::OpenConfigEditor(){
    ConfigEditorDlg dlg(this, cfg_);
    if (dlg.ShowModal() == wxID_OK) {
        RefreshConfigLabel();
        // ggf. Timer neu starten, falls Intervall geändert
        if (ui_timer_.IsRunning()) ui_timer_.Stop();
        ui_timer_.Start(std::max(50, cfg_.update_interval_ms));
    }
}
