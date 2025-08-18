#pragma once
#include <wx/string.h>
#include <vector>

// Kleiner, GUI-freundlicher Logger mit Speicher im RAM.
// Optional kannst du später einen Sink ins GUI (MainFrame) anbinden.
class LoggerService {
public:
    struct Entry {
        wxString timestamp;   // ISO "YYYY-MM-DD HH:MM:SS"
        wxString category;    // z.B. "Config", "Relais", "Test"
        wxString message;     // Freitext
        wxString severity;    // "OK", "INFO", "WARN", "ERROR"
        int      channel = -1; // -1 = keiner, sonst 0..7
        wxString serial;      // Seriennummer (falls vorhanden)
        wxString relay_state; // "ON"/"OFF" oder "-"
    };

    // Hilfsfunktion: aktuelle Zeit im ISO-Format
    static wxString NowIso();

    // Eintrag hinzufügen
    void Log(const wxString& category,
             const wxString& message,
             const wxString& severity,
             int channel = -1,
             const wxString& serial = wxString(),
             const wxString& relay_state = wxString());

    // Speicher löschen
    void Clear() { entries_.clear(); }

    // Zugriff (read-only)
    const std::vector<Entry>& Entries() const { return entries_; }

private:
    std::vector<Entry> entries_;
};
