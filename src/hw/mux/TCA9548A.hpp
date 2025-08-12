#pragma once
#include <string>
#include <cstdint>

/**
 * @brief Minimaler Treiber für TCA9548A I²C-Multiplexer.
 *
 * Funktionen:
 *  - Init(i2c_dev="/dev/i2c-1", addr=0x70)
 *  - Select(channel 0..7)  -> aktiviert genau einen Kanal
 *  - Optional: SelectMask(mask) für mehrere Kanäle (Bitmaske)
 */
class TCA9548A {
public:
    TCA9548A() = default;
    ~TCA9548A();

    // non-copyable, movable
    TCA9548A(const TCA9548A&) = delete;
    TCA9548A& operator=(const TCA9548A&) = delete;
    TCA9548A(TCA9548A&&) noexcept;
    TCA9548A& operator=(TCA9548A&&) noexcept;

    bool Init(const std::string& i2c_dev = "/dev/i2c-1", uint8_t addr = 0x70, std::string* err = nullptr);
    void Close();

    // Aktiviert genau EINEN Kanal (0..7). Andere Kanäle werden deaktiviert.
    bool Select(int channel, std::string* err = nullptr);

    // Aktiviert mehrere Kanäle (Bitmaske; Bit0 -> CH0, ... Bit7 -> CH7).
    bool SelectMask(uint8_t mask, std::string* err = nullptr);

    // Aktuelle Einstellungen (nur lokal gemerkt)
    [[nodiscard]] uint8_t address() const { return addr_; }
    [[nodiscard]] const std::string& device() const { return dev_path_; }
    [[nodiscard]] bool isOpen() const { return fd_ >= 0; }
    [[nodiscard]] uint8_t lastMask() const { return last_mask_; }

private:
    bool writeByte(uint8_t value, std::string* err);

    int         fd_       = -1;
    std::string dev_path_;
    uint8_t     addr_     = 0x70;
    uint8_t     last_mask_= 0x00;
};
