#pragma once
#include <optional>
#include <string>
#include <cstdint>

/**
 * @brief Minimaler ULDAQ-Adapter für MCC/RedLab-USB-DAQs.
 *
 * Features:
 *  - Geräte-Discovery (nimmt das erste gefundene Gerät)
 *  - Connect / Disconnect (RAII-ähnlich)
 *  - Einfache Bereichswahl (±10V, ±5V, 0-5V, etc.)
 *  - Einzelmessung: Read(channel) -> Volt
 *
 * Wichtig:
 *  - Prüfe im Handbuch deines DAQ die gültigen AI-Ranges & Eingangsmodi (SE/DIFF).
 *  - Je nach ULDAQ-Version heißen die Funktionen geringfügig anders (ulAiRead / ulAIn / ulAInScan).
 *    Die Read-Stelle ist unten klar markiert – dort ggf. die passende API wählen.
 */
class RedLabDAQ {
public:
    enum class Range {
        Bip10V,
        Bip5V,
        Uni5V,
        Uni10V
    };

    struct Options {
        Range range = Range::Bip10V;  ///< Standard: ±10V
        bool differential = false;    ///< false = Single-Ended, true = Differential
        int device_index = 0;         ///< Welches Gerät aus der Inventarliste (0 = erstes)
    };

    RedLabDAQ();
    ~RedLabDAQ();

    // Non-copyable, movable
    RedLabDAQ(const RedLabDAQ&) = delete;
    RedLabDAQ& operator=(const RedLabDAQ&) = delete;
    RedLabDAQ(RedLabDAQ&&) noexcept;
    RedLabDAQ& operator=(RedLabDAQ&&) noexcept;

    bool Connect(const Options& opt = {}, std::string* err = nullptr);
    void Disconnect();

    /// Liest eine Einzelmessung in Volt von Kanal ch (0-basiert).
    std::optional<double> Read(int ch, std::string* err = nullptr) const;

    /// Bereich wechseln (wirkt auf folgende Messungen)
    bool SetRange(Range r, std::string* err = nullptr);

    bool IsConnected() const { return connected_; }
    Range CurrentRange() const { return range_; }
    bool IsDifferentialMode() const { return differential_; }

private:
    // Forward-Declare ULDAQ-Typen über void*, damit der Header ohne ULDAQ kompilierbar bleibt
    using DaqDeviceHandle = void*;

    static int  ToUldaqRange(RedLabDAQ::Range r);
    static void SetErr(std::string* err, const std::string& msg);

    // interner Helper: Read-Call der ULDAQ-API (siehe .cpp)
    bool UldaqReadSingle(int ch, double* out_volt, std::string* err) const;

private:
    DaqDeviceHandle handle_ = nullptr;
    bool connected_ = false;
    Range range_ = Range::Bip10V;
    bool differential_ = false;
    int device_index_ = 0;
};
