#include "services/LoggerService.hpp"
#include <wx/datetime.h>

wxString LoggerService::NowIso() {
    return wxDateTime::Now().FormatISOCombined(' ');
}

void LoggerService::Log(const wxString& category,
                        const wxString& message,
                        const wxString& severity,
                        int channel,
                        const wxString& serial,
                        const wxString& relay_state)
{
    Entry e;
    e.timestamp   = NowIso();
    e.category    = category;
    e.message     = message;
    e.severity    = severity;
    e.channel     = channel;
    e.serial      = serial;
    e.relay_state = relay_state;
    entries_.push_back(std::move(e));
}
