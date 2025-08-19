#include "RedLabDAQ.hpp"

#include <sstream>
#include <cstring>
#include <algorithm>

// ---- Hilfen ----------------------------------------------------------------

static std::string UldaqStatusToString(ULSTATUS st) {
    // UL liefert kein universelles getErrMsg-API; wir geben den Code aus.
    std::ostringstream oss;
    oss << "ULSTATUS=" << static_cast<long>(st);
    return oss.str();
}

int RedLabDAQ::PickDeviceIndex(const Options& opt, std::string* err) {
    const unsigned int MAX_DEVICES = 64;
    DaqDeviceDescriptor descs[MAX_DEVICES];
    unsigned int count = MAX_DEVICES;

    ULSTATUS st = ulGetDaqDeviceInventory(ANY_IFC, descs, &count);
    if (st != ERR_NO_ERROR) {
        SetErr(err, "RedLab: ulGetDaqDeviceInventory fehlgeschlagen: " + UldaqStatusToString(st));
        return -1;
    }
    if (count == 0) {
        SetErr(err, "RedLab: kein ULDAQ-Gerät gefunden");
        return -1;
    }

    // 1) Seriennummer bevorzugen
    if (!opt.serial.empty()) {
        for (unsigned int i = 0; i < count; ++i) {
            if (descs[i].UniqueId && opt.serial == descs[i].UniqueId) {
                return static_cast<int>(i);
            }
        }
        SetErr(err, "RedLab: Seriennummer nicht gefunden: " + opt.serial);
        return -1;
    }

    // 2) Produktname
    if (!opt.product_name.empty()) {
        for (unsigned int i = 0; i < count; ++i) {
            if (descs[i].ProductName && opt.product_name == descs[i].ProductName) {
                return static_cast<int>(i);
            }
        }
        SetErr(err, "RedLab: Produktname nicht gefunden: " + opt.product_name);
        return -1;
    }

    // 3) Fallback: device_index
    if (opt.device_index < 0 || static_cast<unsigned int>(opt.device_index) >= count) {
        std::ostringstream oss;
        oss << "RedLab: device_index " << opt.device_index
            << " außerhalb (0.." << (count - 1) << ")";
        SetErr(err, oss.str());
        return -1;
    }
    return opt.device_index;
}

::Range RedLabDAQ::ToUldaqRange(Options::Range r) {
    switch (r) {
        case Options::Range::Bip10V: return BIP10VOLTS;
        case Options::Range::Bip5V:  return BIP5VOLTS;
        case Options::Range::Uni5V:  return UNI5VOLTS;
        case Options::Range::Uni10V: return UNI10VOLTS;
    }
    // Fallback
    return BIP5VOLTS;
}

// ---- Public API -------------------------------------------------------------

bool RedLabDAQ::Connect(const Options& opt, std::string* err) {
    std::scoped_lock lk(m_);

    Disconnect(); // sicherstellen, dass wir „clean“ starten

    int picked = PickDeviceIndex(opt, err);
    if (picked < 0) return false;

    const unsigned int MAX_DEVICES = 64;
    DaqDeviceDescriptor descs[MAX_DEVICES];
    unsigned int count = MAX_DEVICES;
    ULSTATUS st = ulGetDaqDeviceInventory(ANY_IFC, descs, &count);
    if (st != ERR_NO_ERROR) {
        last_status_ = st;
        last_error_  = "ulGetDaqDeviceInventory fehlgeschlagen";
        SetErr(err, last_error_ + ": " + UldaqStatusToString(st));
        return false;
    }

    device_index_  = picked;
    single_ended_  = opt.single_ended;
    range_         = opt.range;

    // Gerät erzeugen + verbinden
    handle_ = ulCreateDaqDevice(descs[device_index_]);
    if (!handle_) {
        last_status_ = ERR_BAD_HANDLE;
        last_error_  = "ulCreateDaqDevice gab null Handle zurück";
        SetErr(err, last_error_);
        return false;
    }

    st = ulConnectDaqDevice(handle_);
    if (st != ERR_NO_ERROR) {
        last_status_ = st;
        last_error_  = "ulConnectDaqDevice fehlgeschlagen";
        ulReleaseDaqDevice(handle_);
        handle_ = nullptr;
        SetErr(err, last_error_ + ": " + UldaqStatusToString(st));
        return false;
    }

    connected_ = true;
    last_status_ = ERR_NO_ERROR;
    last_error_.clear();
    return true;
}

void RedLabDAQ::Disconnect() {
    if (!connected_ && !handle_) return;

    std::scoped_lock lk(m_);
    if (handle_) {
        // Reihenfolge: erst trennen, dann freigeben
        ulDisconnectDaqDevice(handle_);
        ulReleaseDaqDevice(handle_);
        handle_ = nullptr;
    }
    connected_ = false;
    // defensiv: Zustand zurücksetzen
    last_status_ = ERR_NO_ERROR;
    last_error_.clear();
}

bool RedLabDAQ::SetRange(Options::Range r, std::string* /*err*/) {
    std::scoped_lock lk(m_);
    range_ = r;
    return true; // für ulAIn wird Range im Call übergeben
}

std::optional<double> RedLabDAQ::Read(int ch, std::string* err) const {
    std::scoped_lock lk(m_);

    if (!connected_ || !handle_) {
        SetErr(err, "RedLab: nicht verbunden");
        return std::nullopt;
    }

    // Kanalprüfung Single-Ended
    if (single_ended_) {
        if (ch < 0 || ch > 7) {
            std::ostringstream oss;
            oss << "RedLab: Single-Ended Kanal außerhalb (0..7): " << ch;
            SetErr(err, oss.str());
            return std::nullopt;
        }
    } else {
        // Für Vollständigkeit – wird in deinem Projekt nicht verwendet
        if (ch < 0 || ch > 3) {
            std::ostringstream oss;
            oss << "RedLab: Differential Kanal außerhalb (0..3): " << ch;
            SetErr(err, oss.str());
            return std::nullopt;
        }
    }

    double v = 0.0;
    if (!UldaqReadSingle(ch, &v, err)) return std::nullopt;
    return v;
}

bool RedLabDAQ::ReadMany(const std::vector<int>& channels, std::vector<double>* out, std::string* err) const {
    if (!out) { SetErr(err, "RedLab: ReadMany(out) ist null"); return false; }

    std::scoped_lock lk(m_);

    if (!connected_ || !handle_) {
        SetErr(err, "RedLab: nicht verbunden");
        return false;
    }

    out->clear();
    out->reserve(channels.size());

    for (int ch : channels) {
        // Kanalprüfung Single-Ended
        if (single_ended_) {
            if (ch < 0 || ch > 7) {
                std::ostringstream oss;
                oss << "RedLab: Single-Ended Kanal außerhalb (0..7): " << ch;
                SetErr(err, oss.str());
                return false;
            }
        } else {
            if (ch < 0 || ch > 3) {
                std::ostringstream oss;
                oss << "RedLab: Differential Kanal außerhalb (0..3): " << ch;
                SetErr(err, oss.str());
                return false;
            }
        }

        double v = 0.0;
        if (!UldaqReadSingle(ch, &v, err)) return false;
        out->push_back(v);
    }
    return true;
}

// ---- intern ----------------------------------------------------------------

bool RedLabDAQ::UldaqReadSingle(int ch, double* out_volt, std::string* err) const {
    // Wir nutzen ulAIn (Single-Shot). Der Messbereich wird hier übergeben.
    const ::Range urange = ToUldaqRange(range_);

    ULSTATUS st = ulAIn(handle_, ch, urange, out_volt);
    if (st != ERR_NO_ERROR) {
        last_status_ = st;
        std::ostringstream oss;
        oss << "ulAIn fehlgeschlagen: ch=" << ch << " " << UldaqStatusToString(st);
        last_error_ = oss.str();
        SetErr(err, last_error_);
        return false;
    }
    last_status_ = ERR_NO_ERROR;
    last_error_.clear();
    return true;
}
