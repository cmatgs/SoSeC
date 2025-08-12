#include "RelayController.hpp"
#include <cerrno>
#include <cstring>
#include <sstream>

// Hilfsfunktionen
void RelayController::setErr(std::string* err, const std::string& msg) {
    if (err) *err = msg;
}
bool RelayController::checkIndex(size_t index, std::string* err) const {
    if (index >= lines_.size()) {
        setErr(err, "Relais-Index außerhalb des Bereichs");
        return false;
    }
    return true;
}

bool RelayController::readPhysical(size_t index, int* level, std::string* err) const {
    if (!checkIndex(index, err)) return false;
    int v = gpiod_line_get_value(lines_[index]);
    if (v < 0) {
        std::ostringstream oss;
        oss << "gpiod_line_get_value fehlgeschlagen: " << std::strerror(errno);
        setErr(err, oss.str());
        return false;
    }
    *level = v;
    return true;
}

// Konstruktor / Destruktor
RelayController::RelayController(const Config& cfg) : cfg_(cfg) {}
RelayController::~RelayController() { shutdown(); }

// Move
RelayController::RelayController(RelayController&& other) noexcept {
    cfg_ = other.cfg_;
    chip_ = other.chip_;
    lines_ = std::move(other.lines_);
    initialized_ = other.initialized_;
    other.chip_ = nullptr;
    other.initialized_ = false;
}
RelayController& RelayController::operator=(RelayController&& other) noexcept {
    if (this != &other) {
        shutdown();
        cfg_ = other.cfg_;
        chip_ = other.chip_;
        lines_ = std::move(other.lines_);
        initialized_ = other.initialized_;
        other.chip_ = nullptr;
        other.initialized_ = false;
    }
    return *this;
}

// Lifecycle
bool RelayController::init(std::string* err) {
    if (initialized_) return true;
    if (cfg_.pins.empty()) {
        setErr(err, "Keine GPIO-Pins konfiguriert");
        return false;
    }

    chip_ = gpiod_chip_open(cfg_.chip_path.c_str());
    if (!chip_) {
        std::ostringstream oss;
        oss << "Kann Chip nicht öffnen (" << cfg_.chip_path << "): " << std::strerror(errno);
        setErr(err, oss.str());
        return false;
    }

    lines_.resize(cfg_.pins.size(), nullptr);
    for (size_t i = 0; i < cfg_.pins.size(); ++i) {
        gpiod_line* line = gpiod_chip_get_line(chip_, cfg_.pins[i]);
        if (!line) {
            std::ostringstream oss;
            oss << "Kann Line für GPIO" << cfg_.pins[i] << " nicht holen: " << std::strerror(errno);
            setErr(err, oss.str());
            return false;
        }
        lines_[i] = line;

        int initial = cfg_.active_high ? (cfg_.initial_on ? 1 : 0)
                                       : (cfg_.initial_on ? 0 : 1);

        if (gpiod_line_request_output(line, cfg_.consumer.c_str(), initial) < 0) {
            std::ostringstream oss;
            oss << "Line-Request OUTPUT fehlgeschlagen (GPIO" << cfg_.pins[i]
                << "): " << std::strerror(errno);
            setErr(err, oss.str());
            return false;
        }
    }

    initialized_ = true;
    return true;
}

void RelayController::shutdown() {
    for (auto* line : lines_) {
        if (line && gpiod_line_is_requested(line)) {
            gpiod_line_release(line);
        }
    }
    lines_.clear();
    if (chip_) {
        gpiod_chip_close(chip_);
        chip_ = nullptr;
    }
    initialized_ = false;
}

// Einzelne Relais
bool RelayController::set(size_t index, bool on, std::string* err) {
    if (!initialized_) { setErr(err, "Controller nicht initialisiert"); return false; }
    if (!checkIndex(index, err)) return false;
    int phys = cfg_.active_high ? (on ? 1 : 0) : (on ? 0 : 1);
    if (gpiod_line_set_value(lines_[index], phys) < 0) {
        std::ostringstream oss;
        oss << "gpiod_line_set_value fehlgeschlagen (GPIO" << cfg_.pins[index]
            << "): " << std::strerror(errno);
        setErr(err, oss.str());
        return false;
    }
    return true;
}

bool RelayController::get(size_t index, bool* on, std::string* err) const {
    if (!initialized_) { setErr(err, "Controller nicht initialisiert"); return false; }
    if (!on) { setErr(err, "Nullpointer für Ausgabeparameter"); return false; }
    int phys{};
    if (!readPhysical(index, &phys, err)) return false;
    *on = cfg_.active_high ? (phys == 1) : (phys == 0);
    return true;
}

bool RelayController::toggle(size_t index, std::string* err) {
    bool cur{};
    if (!get(index, &cur, err)) return false;
    return set(index, !cur, err);
}

// Alle Relais
bool RelayController::setAll(bool on, std::string* err) {
    for (size_t i = 0; i < lines_.size(); ++i) {
        if (!set(i, on, err)) return false;
    }
    return true;
}

bool RelayController::setMask(uint32_t mask, std::string* err) {
    for (size_t i = 0; i < lines_.size(); ++i) {
        bool on = (mask >> i) & 0x1;
        if (!set(i, on, err)) return false;
    }
    return true;
}
