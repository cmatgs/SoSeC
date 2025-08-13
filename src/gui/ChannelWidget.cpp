#include "ChannelWidget.hpp"
#include <wx/dcbuffer.h>
#include <wx/statline.h>
#include <algorithm>

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

ChannelWidget::ChannelWidget(wxWindow* parent, int channel, SoSeStaApp* app)
: wxStaticBoxSizer(wxVERTICAL, parent, wxString::Format("Kanal %d", channel+1))
, app_(app), ch_(channel)
{
    auto* box = GetStaticBox();

    // ── Kopf: LED links, rechts "SN:" + Entry
    {
        auto* top = new wxBoxSizer(wxHORIZONTAL);
        led_ = new LedCircle(box);
        top->Add(led_, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);

        top->AddStretchSpacer();

        auto* sn_label = new wxStaticText(box, wxID_ANY, "SN:");
        top->Add(sn_label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

        sn_ = new wxTextCtrl(box, wxID_ANY, "", wxDefaultPosition, wxSize(110, -1));
        top->Add(sn_, 0, wxALIGN_CENTER_VERTICAL);

        this->Add(top, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 2);
    }

    // Helper: Key–Value-Zeile (Label links, Wert rechts)
    auto makeRow = [this, box](const wxString& label, wxStaticText*& valueCtrl) {
        auto* row = new wxBoxSizer(wxHORIZONTAL);
        auto* lbl = new wxStaticText(box, wxID_ANY, label);
        valueCtrl = new wxStaticText(box, wxID_ANY, "—");
        row->Add(lbl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
        row->AddStretchSpacer();
        row->Add(valueCtrl, 0, wxALIGN_CENTER_VERTICAL);
        this->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 2);
    };

    // Helper: Abschnitts-Titel (fetter Text, volle Breite)
    auto makeTitle = [this, box](const wxString& title){
        auto* t = new wxStaticText(box, wxID_ANY, title);
        auto f = t->GetFont(); f.MakeBold(); t->SetFont(f);
        this->Add(t, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
    };

    // Helper: Trennlinie
    auto makeSeparator = [this, box](){
        auto* line = new wxStaticLine(box);
        this->Add(line, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
    };

    // Status/Relais
    makeRow("Status:", status_);
    status_->SetForegroundColour(wxColour(120,120,120));
    makeRow("Relais:", relay_);

    makeSeparator();
    makeTitle("Sensorwerte");
    makeRow("Strom:",    current_);
    makeRow("Spannung:", voltage_);
    makeRow("Signal:",   redlab_);

    makeSeparator();
    makeTitle("Fehlerzähler");
    makeRow("Versorgung:", err_supply_);
    makeRow("Signal:",     err_signal_);
    makeRow("Strom:",      err_current_);

    err_supply_->SetLabel("0");
    err_signal_->SetLabel("0");
    err_current_->SetLabel("0");
}

void ChannelWidget::UpdateFrom(const SensorData& d){
    // Messwerte
    current_->SetLabel(wxString::Format("%.2f mA", d.current_mA));
    voltage_->SetLabel(wxString::Format("%.2f V",  d.bus_V));
    redlab_->SetLabel(wxString::Format("%.2f V",   d.redlab_V));

    // Relaiszustand (Kanal 0–7 -> Relais 0–3)
    const int relay_idx = d.channel / 2;
    const bool rstate = app_->hw->Relays().GetState(relay_idx);
    relay_->SetLabel(rstate ? "ON" : "OFF");

    // Status + LED
    LedCircle::Color c = LedCircle::Color::Gray;
    wxString text = "—";
    if (!d.present || !d.supply_ok) { c = LedCircle::Color::Purple; text = "unbekannt"; }
    else if (d.signal_ok)            { c = LedCircle::Color::Green;  text = "OK"; }
    else if (d.redlab_V != 0.0)      { c = LedCircle::Color::Orange; text = "Warnung"; }
    else                             { c = LedCircle::Color::Red;    text = "Fehler"; }

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
    app_->serial_numbers[d.channel] = sn_->GetValue().ToStdString();
}

void ChannelWidget::SetRelayState(bool on){
    relay_->SetLabel(on ? "ON" : "OFF");
}

void ChannelWidget::DisableSerialInput(){
    sn_->Enable(false);
}
