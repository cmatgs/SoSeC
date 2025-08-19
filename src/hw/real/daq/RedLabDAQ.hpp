#pragma once

#include <string>
#include <optional>
#include <vector>
#include <mutex>

// ULDAQ
#include <uldaq.h>

/**
 * RedLabDAQ – einfacher, threadsicherer Wrapper für ULDAQ (z. B. USB-1208FS-Plus)
 * - Verwendet ulAIn (Single-Shot), Single-Ended (0..7)
 * - Standard-Messbereich: ±5 V  (BIP5VOLTS)
 * - Thread-safe via Mutex
 * - Saubere Fehlerrückgabe & Diagnose
 */
class RedLabDAQ {
public:
    struct Options {
        int device_index = 0;          // Welches Gerät aus dem Inventory (fallback)
        std::string serial;            // Optional: präferierte Seriennummer (UniqueId)
        std::string product_name;      // Optional: präferierter Produktname
        enum class Range { Bip10V, Bip5V, Uni5V, Uni10V };
        Range range = Range::Bip5V;    // Standard = ±5 V
        bool single_ended = true;      // In deinem Projekt: immer true
    };

    RedLabDAQ() = default;
    ~RedLabDAQ() { Disconnect(); }

    // Verbindung herstellen (wählt Gerät per serial/product_name oder fallback auf device_index)
    bool Connect(const Options& opt, std::string* err = nullptr);

    // Verbindung trennen (idempotent)
    void Disconnect();

    bool IsConnected() const { return connected_; }

    // Bereich setzen (wirken bei ulAIn als Parameter; SetRange speichert nur lokal)
    bool SetRange(Options::Range r, std::string* err = nullptr);

    // Einzelmessung eines Kanals (Single-Ended: gültig 0..7)
    std::optional<double> Read(int ch, std::string* err = nullptr) const;

    // Mehrere Kanäle in einem Rutsch (unter einem Lock)
    bool ReadMany(const std::vector<int>& channels, std::vector<double>* out, std::string* err = nullptr) const;

    // Diagnose
    ULSTATUS last_status() const { std::scoped_lock lk(m_); return last_status_; }
    std::string last_error() const { std::scoped_lock lk(m_); return last_error_; }

private:
    static ::Range ToUldaqRange(Options::Range r);
    bool UldaqReadSingle(int ch, double* out_volt, std::string* err) const;
    static void SetErr(std::string* err, const std::string& msg) { if (err) *err = msg; }

    // Gerät auswählen (über Inventory) – gibt Index im Inventory zurück oder -1 bei Fehler
    static int PickDeviceIndex(const Options& opt, std::string* err);

private:
    // ULDAQ
    DaqDeviceHandle handle_ = nullptr;

    // Zustand
    bool connected_ = false;
    Options::Range range_ = Options::Range::Bip5V;
    int device_index_ = 0;
    bool single_ended_ = true; // dein Setup

    // Diagnose
    mutable ULSTATUS last_status_ = ERR_NO_ERROR;
    mutable std::string last_error_;

    // Thread-Schutz
    mutable std::mutex m_;
};
