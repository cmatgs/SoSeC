#pragma once
#include <wx/wx.h>
#include "App.hpp"

class LedCircle : public wxPanel {
public:
    enum class Color { Gray, Green, Red, Orange, Purple };
    explicit LedCircle(wxWindow* parent);
    void Set(Color c);
private:
    Color color_ = Color::Gray;
    void OnPaint(wxPaintEvent& evt);
    wxDECLARE_EVENT_TABLE();
};

class ChannelWidget : public wxStaticBoxSizer {
public:
    ChannelWidget(wxWindow* parent, int channel, SoSeStaApp* app);

    void UpdateFrom(const SensorData& d);
    void SetRelayState(bool on);
    void DisableSerialInput();

private:
    SoSeStaApp* app_;
    int ch_;
    LedCircle* led_;

    // Werte rechts
    wxStaticText* current_   = nullptr;
    wxStaticText* voltage_   = nullptr;
    wxStaticText* redlab_    = nullptr;
    wxStaticText* relay_     = nullptr;
    wxStaticText* status_    = nullptr;

    // Fehlerzähler – eigene Zeilen
    wxStaticText* err_supply_  = nullptr;
    wxStaticText* err_signal_  = nullptr;
    wxStaticText* err_current_ = nullptr;

    // Seriennummer-Input
    wxTextCtrl*   sn_ = nullptr;
};
