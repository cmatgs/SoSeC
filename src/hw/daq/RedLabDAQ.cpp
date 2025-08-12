#include "RedLabDAQ.hpp"
#include <sstream>
#include <cstring>

// === ULDAQ-Header ===
// Stelle sicher, dass uldaq installiert und im Include-Pfad ist (z. B. /usr/local/include).
#include <uldaq.h>

// Kleine Guard-Makros: Prüfen UL-Returncodes
static inline bool UL_OK(ULSTATUS s) { return s == ERR_NO_ERROR; }

RedLabDAQ::RedLabDAQ() = default;
RedLabDAQ::~RedLabDAQ() { Disconnect(); }

RedLabDAQ::RedLabDAQ(RedLabDAQ&& other) noexcept {
    handle_     = other.handle_;
    connected_  = other.connected_;
    range_      = other.range_;
    differential_ = other.differential_;
    device_index_ = other.device_index_;
    other.handle_ = nullptr;
    other.connected_ = false;
}
RedLabDAQ& RedLabDAQ::operator=(RedLabDAQ&& other) noexcept {
    if (this != &other) {
        Disconnect();
        handle_     = other.handle_;
        connected_  = other.connected_;
        range_      = other.range_;
        differential_= other.differential_;
        device_index_= other.device_index_;
        other.handle_ = nullptr;
        other.connected_ = false;
    }
    return *this;
}

void RedLabDAQ::SetErr(std::string* err, const std::string& msg) {
    if (err) *err = msg;
}

int RedLabDAQ::ToUldaqRange(RedLabDAQ::Range r) {
    switch (r) {
        case Range::Bip10V: return BIP10VOLTS;
        case Range::Bip5V:  return BIP5VOLTS;
        case Range::Uni5V:  return UNI5VOLTS;
        case Range::Uni10V: return UNI10VOLTS;
        default:            return BIP10VOLTS;
    }
}

bool RedLabDAQ::Connect(Options opt, std::string* err) {
    Disconnect();
    range_        = opt.range;
    differential_ = opt.differential;
    device_index_ = opt.device_index;

    DaqDeviceDescriptor descs[16];
    unsigned int numDevs = 16;
    ULSTATUS st = ulGetDaqDeviceInventory(ANY_IFC, descs, &numDevs);
    if (!UL_OK(st) || numDevs == 0) {
        std::ostringstream oss;
        oss << "ulGetDaqDeviceInventory: kein Gerät gefunden (status=" << st << ")";
        SetErr(err, oss.str());
        return false;
    }
    if (device_index_ < 0 || static_cast<unsigned>(device_index_) >= numDevs) {
        std::ostringstream oss;
        oss << "Geräteindex " << device_index_ << " außerhalb 0.." << (numDevs-1);
        SetErr(err, oss.str());
        return false;
    }

    // Gerät erzeugen
    handle_ = ulCreateDaqDevice(&descs[device_index_]);
    if (!handle_) {
        SetErr(err, "ulCreateDaqDevice: Handle ist null");
        return false;
    }

    // Verbinden
    st = ulConnectDaqDevice(handle_);
    if (!UL_OK(st)) {
        std::ostringstream oss;
        oss << "ulConnectDaqDevice fehlgeschlagen (status=" << st << ")";
        SetErr(err, oss.str());
        ulReleaseDaqDevice(handle_);
        handle_ = nullptr;
        return false;
    }

    // Optional: AI-Info auslesen, unterstützte Ranges checken (kannst du erweitern)
    // AiInfo aiInfo;
    // ulAIGetInfo(handle_, &aiInfo); // je nach UL-Version; ansonsten ulGetConfig + CONFIG_xxx

    connected_ = true;
    return true;
}

void RedLabDAQ::Disconnect() {
    if (handle_) {
        // Graceful disconnect
        ulDisconnectDaqDevice(handle_);
        ulReleaseDaqDevice(handle_);
        handle_ = nullptr;
    }
    connected_ = false;
}

bool RedLabDAQ::SetRange(Range r, std::string* err) {
    if (!connected_) { SetErr(err, "RedLab: nicht verbunden"); return false; }
    range_ = r;
    // Viele Geräte übernehmen den Range-Parameter direkt im Read-Call.
    // Bei Geräten, die persistenten Range je Kanal speichern, müsste man hier ulAISetConfig(...) o.ä. nutzen.
    return true;
}

std::optional<double> RedLabDAQ::Read(int ch, std::string* err) const {
    if (!connected_) { SetErr(err, "RedLab: nicht verbunden"); return std::nullopt; }
    if (ch < 0) { SetErr(err, "RedLab: negativer Kanalindex"); return std::nullopt; }

    double v{};
    if (!UldaqReadSingle(ch, &v, err)) return std::nullopt;
    return v;
}

bool RedLabDAQ::UldaqReadSingle(int ch, double* out_volt, std::string* err) const {
    // --- WICHTIG ---
    // Je nach ULDAQ-Version/Modell:
    //  * ulAiRead(handle, channel, inputMode, range, flags, &doubleVolt)
    //  * oder ulAIn(handle, channel, range, &doubleVolt) (ältere/andere Signatur)
    //
    // Prüfe das passende Beispiel im uldaq-Repo und wähle die korrekte Funktion.
    // Unten sind beide Varianten skizziert. Kommentiere die unpassende aus.

    ULSTATUS st{ERR_NO_ERROR};

    // Variante A: Moderne Signatur mit InputMode/Flags (häufig bei UL for Linux):
    {
        // InputMode: AI_SINGLE_ENDED oder AI_DIFFERENTIAL
        AININPUTMODE mode = differential_ ? AI_DIFFERENTIAL : AI_SINGLE_ENDED;
        long flags = 0; // z. B. AI_DEFAULT
        st = ulAiRead(reinterpret_cast<DaqDeviceHandle>(handle_),
                      ch,
                      mode,
                      static_cast<Range>(ToUldaqRange(range_)),
                      flags,
                      out_volt);
        if (UL_OK(st)) return true;
        // Wenn dein Build ulAiRead nicht kennt, kommentiere diesen Block aus.
    }

    // Variante B: Einfache Signatur (manche Beispiele/Modelle):
    // {
    //     st = ulAIn(reinterpret_cast<DaqDeviceHandle>(handle_),
    //                ch,
    //                static_cast<Range>(ToUldaqRange(range_)),
    //                out_volt);
    //     if (UL_OK(st)) return true;
    // }

    std::ostringstream oss;
    oss << "ULDAQ Read fehlgeschlagen (status=" << st << ") für Kanal " << ch;
    SetErr(err, oss.str());
    return false;
}
