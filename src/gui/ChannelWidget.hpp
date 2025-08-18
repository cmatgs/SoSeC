#pragma once

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <functional>
#include <vector>
#include <array>
#include <string>
#include <span>
#include "app/data/SensorData.hpp"

// ── Kleine LED-Panel-Klasse ─────────────────────────────
class LedCircle : public wxPanel {
public:
    enum class Color { Gray, Green, Red, Orange, Purple };

    explicit LedCircle(wxWindow* parent);
    void Set(Color c);

private:
    Color color_ = Color::Gray;

    void OnPaint(wxPaintEvent&);

    wxDECLARE_EVENT_TABLE();
};

// ── Kanal-Widget (als Sizer!) ───────────────────────────
class ChannelWidget : public wxStaticBoxSizer {
public:
    // getRelayState(idx) -> true wenn Relais idx (0..3) an ist
    ChannelWidget(wxWindow* parent,
                  int channel,
                  std::function<bool(int)> getRelayState,
                  std::vector<std::string>& serial_numbers);

    ChannelWidget(wxWindow* parent,
                  int channel,
                  std::function<bool(int)> getRelayState,
                  std::array<std::string, 8>& serial_numbers);

    void UpdateFrom(const SensorData& d);
    void SetRelayState(bool on);
    void DisableSerialInput();

private:
    int ch_ = -1;

    // UI-Elemente (alle unter der StaticBox)
    LedCircle*     led_         = nullptr;
    wxTextCtrl*    sn_          = nullptr;

    wxStaticText*  status_      = nullptr;
    wxStaticText*  relay_       = nullptr;

    // Werte
    wxStaticText*  current_     = nullptr;
    wxStaticText*  voltage_     = nullptr;
    wxStaticText*  redlab_      = nullptr;

    // Fehlerzähler
    wxStaticText*  err_supply_  = nullptr;
    wxStaticText*  err_signal_  = nullptr;
    wxStaticText*  err_current_ = nullptr;

    // Logik-Hooks
    std::function<bool(int)> getRelayState_;
    std::span<std::string>   serial_numbers_; // Ansicht auf vector/array
};
