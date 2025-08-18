// src/config/ConfigHardware.hpp
#pragma once
#include <array>
#include <cstdint>
#include <string>

// Alle plattformspezifischen Hardware-Parameter zentral.

struct ConfigHardware {
    // ── I2C (z.B. für Sensoren, MUX, INA219) ───────────────────────────────
    struct I2C {
        int    retries  = 3;     // Anzahl Wiederholungen bei Fehlern
        double delay_s  = 0.1;   // Wartezeit zwischen Versuchen (Sekunden)
    } i2c;

    // ── INA219-Strom-/Spannungssensor ──────────────────────────────────────
    struct INA219 {
        std::string calibration    = "16V_400mA"; // Kalibrierprofil
        int         retries        = 3;
        double      retry_delay_s  = 0.1;
    } ina219;

    // ── RedLab (DAQ) ───────────────────────────────────────────────────────
    struct RedLab {
        int    reconnect_retries   = 3;
        double reconnect_delay_s   = 0.5;
    } redlab;

    // ── Status-LED / WS281x (falls genutzt) ───────────────────────────────
    struct LEDStrip {
        int   pin        = 10;       // GPIO (BCM)
        int   channel    = 0;        // PWM/WS281x Channel
        int   count      = 8;        // LED-Anzahl
        int   freq_hz    = 800000;   // 800 kHz
        int   dma        = 10;       // DMA Channel
        int   brightness = 255;      // 0..255
        bool  invert     = false;    // invertiertes Signal
    } led;

    // ── Relais-Ausgänge (BCM-GPIOs) ────────────────────────────────────────
    // Reihenfolge frei wählbar; hier aus deiner Python-Config übernommen.
    std::array<int, 4> relay_pins { 14, 15, 18, 23 };

    // Hilfen 
    constexpr std::size_t NumRelays() const noexcept { return relay_pins.size(); }
};
