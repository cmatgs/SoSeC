#include "LEDStrip.hpp"
#include <cstring> // memset

// Hilfsfunktionen
uint32_t LEDStrip::packColor(uint8_t r, uint8_t g, uint8_t b) {
    return (static_cast<uint32_t>(r) << 16) |
           (static_cast<uint32_t>(g) << 8)  |
           (static_cast<uint32_t>(b));
}

bool LEDStrip::checkIndex(int idx, int max, std::string* err) {
    if (idx < 0 || idx >= max) {
        if (err) *err = "LED-Index außerhalb des gültigen Bereichs";
        return false;
    }
    return true;
}

// Konstruktor / Destruktor
LEDStrip::LEDStrip(const Config& cfg) : cfg_(cfg) {
    std::memset(&led_, 0, sizeof(led_));
    led_.freq   = cfg_.freq;
    led_.dmanum = cfg_.dma_channel;
    ws2811_channel_t& ch = led_.channel[cfg_.channel_index];
    ch.gpionum    = cfg_.gpio_pin;
    ch.count      = cfg_.led_count;
    ch.invert     = cfg_.invert;
    ch.brightness = cfg_.brightness;
    ch.strip_type = cfg_.strip_type;
}

LEDStrip::~LEDStrip() {
    shutdown();
}

// Move-Semantik
void LEDStrip::moveFrom(LEDStrip& other) noexcept {
    cfg_ = other.cfg_;
    led_ = other.led_;
    initialized_ = other.initialized_;
    other.initialized_ = false;
    std::memset(&other.led_, 0, sizeof(other.led_));
}

LEDStrip::LEDStrip(LEDStrip&& other) noexcept {
    moveFrom(other);
}

LEDStrip& LEDStrip::operator=(LEDStrip&& other) noexcept {
    if (this != &other) {
        shutdown();
        moveFrom(other);
    }
    return *this;
}

// Lifecycle
bool LEDStrip::init(std::string* err) {
    if (initialized_) return true;
    if (auto rc = ws2811_init(&led_); rc != WS2811_SUCCESS) {
        if (err) *err = std::string("ws2811_init fehlgeschlagen: ") + ws2811_get_return_t_str(rc);
        return false;
    }
    initialized_ = true;
    return true;
}

void LEDStrip::shutdown() {
    if (initialized_) {
        ws2811_fini(&led_);
        initialized_ = false;
    }
}

// Pixelmanipulation
bool LEDStrip::setPixel(int index, uint8_t r, uint8_t g, uint8_t b, std::string* err) {
    if (!initialized_) { if (err) *err = "LEDStrip nicht initialisiert"; return false; }
    auto& ch = led_.channel[cfg_.channel_index];
    if (!checkIndex(index, ch.count, err)) return false;
    ch.leds[index] = packColor(r,g,b);
    return true;
}

bool LEDStrip::fill(uint8_t r, uint8_t g, uint8_t b, std::string* err) {
    if (!initialized_) { if (err) *err = "LEDStrip nicht initialisiert"; return false; }
    auto& ch = led_.channel[cfg_.channel_index];
    uint32_t c = packColor(r,g,b);
    for (int i = 0; i < ch.count; ++i) ch.leds[i] = c;
    return true;
}

bool LEDStrip::clear(std::string* err) {
    return fill(0,0,0, err);
}

bool LEDStrip::show(std::string* err) {
    if (!initialized_) { if (err) *err = "LEDStrip nicht initialisiert"; return false; }
    if (auto rc = ws2811_render(&led_); rc != WS2811_SUCCESS) {
        if (err) *err = std::string("ws2811_render fehlgeschlagen: ") + ws2811_get_return_t_str(rc);
        return false;
    }
    return true;
}

// Einstellungen
bool LEDStrip::setBrightness(uint8_t value, std::string* err) {
    if (!initialized_) { if (err) *err = "LEDStrip nicht initialisiert"; return false; }
    led_.channel[cfg_.channel_index].brightness = value;
    return true;
}
