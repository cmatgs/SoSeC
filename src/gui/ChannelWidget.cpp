#include "gui/ChannelWidget.hpp"
#include "app/data/SensorData.hpp"
#include <wx/valnum.h>
#include <algorithm>
#include <wx/dcbuffer.h>
#include <wx/statline.h>

// ── LedCircle ─────────────────────────────────────────────
wxBEGIN_EVENT_TABLE(LedCircle, wxPanel)
    EVT_PAINT(LedCircle::OnPaint)
wxEND_EVENT_TABLE()

LedCircle::LedCircle(wxWindow* parent)
  : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(24,24))
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void LedCircle::Set(Color c){ color_ = c; Refresh(); }

void LedCircle::OnPaint(wxPaintEvent&){
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    wxColour col(160,160,160);
    switch(color_){
        case Color::Green:  col=wxColour(0,160,0); break;
        case Color::Red:    col=wxColour(200,0,0); break;
        case Color::Orange: col=wxColour(230,140,0); break;
        case Color::Purple: col=wxColour(120,0,160); break;
        case Color::Gray:   default: break;
    }
    dc.SetBrush(wxBrush(col));
    dc.SetPen(*wxTRANSPARENT_PEN);
    wxSize s = GetClientSize();
    dc.DrawCircle(wxPoint(s.x/2, s.y/2), std::min(s.x,s.y)/2 - 2);
}

// Gemeinsamer Aufbau (Controls IMMER mit 'parent' erzeugen!)
static void BuildChannelUi(wxStaticBoxSizer* self, wxWindow* parent,
                           LedCircle*& led_, wxTextCtrl*& sn_,
                           wxStaticText*& status_, wxStaticText*& relay_,
                           wxStaticText*& current_, wxStaticText*& voltage_, wxStaticText*& redlab_,
                           wxStaticText*& err_supply_, wxStaticText*& err_signal_, wxStaticText*& err_current_)
{
    // Kopf: LED links, rechts "SN:" + Entry
    {
        auto* top = new wxBoxSizer(wxHORIZONTAL);
        led_ = new LedCircle(parent);
        top->Add(led_, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);

        top->AddStretchSpacer();

        auto* sn_label = new wxStaticText(parent, wxID_ANY, "SN:");
        top->Add(sn_label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

        sn_ = new wxTextCtrl(parent, wxID_ANY, "", wxDefaultPosition, wxSize(110, -1));
        top->Add(sn_, 0, wxALIGN_CENTER_VERTICAL);

        self->Add(top, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 2);
    }

    auto makeRow = [self, parent](const wxString& label, wxStaticText*& valueCtrl) {
        auto* row = new wxBoxSizer(wxHORIZONTAL);
        auto* lbl = new wxStaticText(parent, wxID_ANY, label);
        valueCtrl = new wxStaticText(parent, wxID_ANY, "—");
        row->Add(lbl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
        row->AddStretchSpacer();
        row->Add(valueCtrl, 0, wxALIGN_CENTER_VERTICAL);
        self->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 2);
    };
    auto makeTitle = [self, parent](const wxString& title){
        auto* t = new wxStaticText(parent, wxID_ANY, title);
        auto f = t->GetFont(); f.MakeBold(); t->SetFont(f);
        self->Add(t, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
    };
    auto makeSeparator = [self, parent](){
        auto* line = new wxStaticLine(parent);
        self->Add(line, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
    };

    // Status/Relais
    makeRow(wxString::FromUTF8("Status:"), status_);
    status_->SetForegroundColour(wxColour(120,120,120));
    makeRow(wxString::FromUTF8("Relais:"), relay_);

    makeSeparator();
    makeTitle(wxString::FromUTF8("Sensorwerte"));
    makeRow(wxString::FromUTF8("Strom:"),    current_);
    makeRow(wxString::FromUTF8("Spannung:"), voltage_);
    makeRow(wxString::FromUTF8("Signal:"),   redlab_);

    makeSeparator();
    makeTitle(wxString::FromUTF8("Fehleranzeige"));
    makeRow(wxString::FromUTF8("Versorgung:"), err_supply_);
    makeRow(wxString::FromUTF8("Signal:"),     err_signal_);
    makeRow(wxString::FromUTF8("Strom:"),      err_current_);

    err_supply_->SetLabel("0");
    err_signal_->SetLabel("0");
    err_current_->SetLabel("0");
}

// ── ChannelWidget (vector) ────────────────────────────────
ChannelWidget::ChannelWidget(wxWindow* parent,
                             int channel,
                             std::function<bool(int)> getRelayState,
                             std::vector<std::string>& serial_numbers)
: wxStaticBoxSizer(wxVERTICAL, parent, wxString::Format("Kanal %d", channel+1))
, ch_(channel)
, getRelayState_(std::move(getRelayState))
, serial_numbers_(std::span<std::string>(serial_numbers))
{
    BuildChannelUi(this, parent, led_, sn_, status_, relay_,
                   current_, voltage_, redlab_, err_supply_, err_signal_, err_current_);
}

// ── ChannelWidget (array<8>) ─────────────────────────────
ChannelWidget::ChannelWidget(wxWindow* parent,
                             int channel,
                             std::function<bool(int)> getRelayState,
                             std::array<std::string, 8>& serial_numbers)
: wxStaticBoxSizer(wxVERTICAL, parent, wxString::Format("Kanal %d", channel+1))
, ch_(channel)
, getRelayState_(std::move(getRelayState))
, serial_numbers_(std::span<std::string>(serial_numbers))
{
    BuildChannelUi(this, parent, led_, sn_, status_, relay_,
                   current_, voltage_, redlab_, err_supply_, err_signal_, err_current_);
}

// ── Updates ───────────────────────────────────────────────
void ChannelWidget::UpdateFrom(const SensorData& d){
    // Messwerte
    current_->SetLabel(wxString::Format("%.2f mA", d.current_mA));
    voltage_->SetLabel(wxString::Format("%.2f V",  d.bus_V));
    redlab_->SetLabel(wxString::Format("%.2f V",   d.redlab_V));

    // Relaiszustand (Kanal 0–7 -> Relais 0–3)
    const int relay_idx = d.channel / 2;
    bool rstate = false;
    if (getRelayState_) rstate = getRelayState_(relay_idx);
    relay_->SetLabel(rstate ? "ON" : "OFF");

    // Status + LED
    LedCircle::Color c = LedCircle::Color::Gray;
    wxString text = "—";
    if (!d.present || !d.supply_ok) { c = LedCircle::Color::Purple; text = wxString::FromUTF8("unbekannt"); }
    else if (d.signal_ok)            { c = LedCircle::Color::Green;  text = "OK"; }
    else if (d.redlab_V != 0.0f)     { c = LedCircle::Color::Orange; text = wxString::FromUTF8("Warnung"); }
    else                             { c = LedCircle::Color::Red;    text = wxString::FromUTF8("Fehler"); }

    status_->SetLabel(text);
    status_->SetForegroundColour(
        c==LedCircle::Color::Green ? wxColour(0,160,0) :
        c==LedCircle::Color::Red   ? wxColour(200,0,0) :
        c==LedCircle::Color::Orange? wxColour(230,140,0) :
        c==LedCircle::Color::Purple? wxColour(120,0,160) : wxColour(120,120,120)
    );
    led_->Set(c);

    // Fehlerzähler
    err_supply_->SetLabel(wxString::Format("%d", d.supply_error_counter));
    err_signal_->SetLabel(wxString::Format("%d", d.signal_error_counter));
    err_current_->SetLabel(wxString::Format("%d", d.current_error_counter));

    // SN übernehmen
    if (ch_ >= 0 && ch_ < static_cast<int>(serial_numbers_.size()))
        serial_numbers_[static_cast<size_t>(ch_)] = sn_->GetValue().ToStdString();
}

void ChannelWidget::SetRelayState(bool on){
    relay_->SetLabel(on ? "ON" : "OFF");
}

void ChannelWidget::DisableSerialInput(){
    sn_->Enable(false);
}
