#pragma once
#include <gpiod.h>
#include <string>
#include <vector>
#include <cstdint>

/**
 * @brief GPIO-Relaissteuerung via libgpiod (v1 API).
 *
 * - RAII: init()/shutdown() über ctor/dtor abgesichert
 * - Konfiguration für Chip, Pins, Polarity (active-high/low), Initialzustand
 * - Einzelnes Relais: set/get/toggle
 * - Alle Relais: setAll(), setMask()
 *
 * Voraussetzungen:
 * - Paket: libgpiod-dev
 * - Benutzer in Gruppe "gpio" (oder binary als root ausführen)
 * - Standard-Chip auf RPi: /dev/gpiochip0
 */
class RelayController {
public:
    struct Config {
        std::string chip_path   = "/dev/gpiochip0"; ///< Pfad zum gpiochip
        std::vector<unsigned> pins;                 ///< BCM-GPIO-Nummern für die Relais
        bool active_high        = true;             ///< true: HIGH = Relais EIN
        bool initial_on         = false;            ///< Initialzustand beim Request
        std::string consumer    = "sosesta-relay";  ///< Name im Kernel (debug)
    };

    explicit RelayController(const Config& cfg);
    ~RelayController();

    // Non-copyable
    RelayController(const RelayController&) = delete;
    RelayController& operator=(const RelayController&) = delete;

    // Movable
    RelayController(RelayController&& other) noexcept;
    RelayController& operator=(RelayController&& other) noexcept;

    // Lifecycle
    bool init(std::string* err = nullptr);
    void shutdown();

    // Einzelne Relais
    bool set(size_t index, bool on, std::string* err = nullptr);
    bool get(size_t index, bool* on, std::string* err = nullptr) const;
    bool toggle(size_t index, std::string* err = nullptr);

    // Alle Relais
    bool setAll(bool on, std::string* err = nullptr);
    bool setMask(uint32_t mask, std::string* err = nullptr); // Bit i → Relais i

    // Infos
    size_t count() const { return lines_.size(); }
    bool isInitialized() const { return initialized_; }
    const std::vector<unsigned>& pins() const { return cfg_.pins; }

private:
    bool checkIndex(size_t index, std::string* err) const;
    static void setErr(std::string* err, const std::string& msg);

    // internes Lesen eines Pins (physischer Pegel 0/1)
    bool readPhysical(size_t index, int* level, std::string* err) const;

private:
    Config cfg_;
    gpiod_chip* chip_ = nullptr;
    std::vector<gpiod_line*> lines_;
    bool initialized_ = false;
};
