#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <ws2811.h>

/**
 * @brief C++-Wrapper für die rpi_ws281x LED-Strip Bibliothek.
 *
 * Bietet RAII-Handling (Init/Fini), einfache Pixel- und Füllfunktionen,
 * Helligkeitssteuerung sowie Fehlerbehandlung.
 *
 * Achtung:
 *  - Standard-Pin ist GPIO 18 (PWM).
 *  - Kollisionen mit Audio vermeiden.
 *  - Auf manchen Systemen ist root-Recht oder spezielle Rechte-Config nötig.
 */
class LEDStrip {
public:
    struct Config {
        int gpio_pin      = 18;                  ///< Standard-PWM Pin für WS281x
        int led_count     = 8;                   ///< Anzahl der LEDs im Strip
        int dma_channel   = 10;                  ///< DMA-Kanal
        int invert        = 0;                   ///< 0 = normal, 1 = invertiert
        int brightness    = 255;                 ///< Helligkeit 0..255
        uint32_t strip_type = WS2811_STRIP_GRB;  ///< Farb-Layout
        int freq          = WS2811_TARGET_FREQ;  ///< Frequenz (Hz)
        int channel_index = 0;                   ///< Channel 0 oder 1
    };

    explicit LEDStrip(const Config& cfg);
    ~LEDStrip();

    // Non-copyable
    LEDStrip(const LEDStrip&) = delete;
    LEDStrip& operator=(const LEDStrip&) = delete;

    // Movable
    LEDStrip(LEDStrip&& other) noexcept;
    LEDStrip& operator=(LEDStrip&& other) noexcept;

    // Lebenszyklus
    bool init(std::string* err = nullptr);
    void shutdown();

    // Pixelmanipulation
    bool setPixel(int index, uint8_t r, uint8_t g, uint8_t b, std::string* err = nullptr);
    bool fill(uint8_t r, uint8_t g, uint8_t b, std::string* err = nullptr);
    bool clear(std::string* err = nullptr);
    bool show(std::string* err = nullptr);

    // Einstellungen
    bool setBrightness(uint8_t value, std::string* err = nullptr);

    // Infos
    int  count() const { return cfg_.led_count; }
    int  gpioPin() const { return cfg_.gpio_pin; }
    int  brightness() const { return led_.channel[cfg_.channel_index].brightness; }
    bool isInitialized() const { return initialized_; }

private:
    static uint32_t packColor(uint8_t r, uint8_t g, uint8_t b);
    static bool checkIndex(int idx, int max, std::string* err);
    void moveFrom(LEDStrip& other) noexcept;

    Config cfg_;
    ws2811_t led_{};
    bool initialized_ = false;
};
