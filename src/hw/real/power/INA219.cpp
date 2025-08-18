#include "INA219.hpp"
#include <sstream>

INA219Manager::INA219Manager(const Config& cfg)
: cfg_(cfg)
, ina_(cfg_.shunt_ohms, cfg_.max_expected_amps) // regisin-Konstruktor: (shunt_ohms, max_expected_amps)
{}

INA219Manager::~INA219Manager() {
    Shutdown();
}

INA219Manager::INA219Manager(INA219Manager&& other) noexcept
: cfg_(other.cfg_)
, ina_(other.cfg_.shunt_ohms, other.cfg_.max_expected_amps)
, initialized_(other.initialized_)
{
    // Hinweis: regisin/ina219 hält i. d. R. keine OS-Handles offen,
    // daher reicht ein Neuaufsetzen per Config. Falls nötig, könnte man hier
    // neu konfigurieren, wenn 'initialized_' true ist.
    other.initialized_ = false;
}

INA219Manager& INA219Manager::operator=(INA219Manager&& other) noexcept {
    if (this != &other) {
        Shutdown();
        cfg_ = other.cfg_;
        // re-konstruiere die INA219-Instanz mit den neuen Parametern
        INA219 tmp(cfg_.shunt_ohms, cfg_.max_expected_amps);
        ina_ = std::move(tmp);
        initialized_ = other.initialized_;
        other.initialized_ = false;
    }
    return *this;
}

void INA219Manager::setErr(std::string* err, const std::string& msg) {
    if (err) *err = msg;
}

bool INA219Manager::Init(std::string* err) {
    if (initialized_) return true;

    // Falls die verwendete regisin-Lib die Adresse/BUS setzen kann, hier tun.
    // Viele Varianten nutzen fest /dev/i2c-1 und 0x40; einige haben setAddress()/setBus().
    // Beispiel (nur nutzen, wenn vorhanden):
    // ina_.setAddress(cfg_.i2c_address);
    // ina_.setBus(cfg_.i2c_bus);

    // Konfiguration (Range/Gain/ADC)
    // Rückgabewert ist bei regisin ein bool (true = ok).
    if (!ina_.configure(cfg_.range, cfg_.gain, cfg_.bus_adc, cfg_.shunt_adc)) {
        setErr(err, "INA219: configure() fehlgeschlagen");
        return false;
    }

    initialized_ = true;
    return true;
}

std::optional<std::tuple<double,double,double>> INA219Manager::Read(std::string* err) const {
    if (!initialized_) {
        setErr(err, "INA219: nicht initialisiert");
        return std::nullopt;
    }

    // regisin/ina219 API:
    //  - voltage() -> V
    //  - current() -> mA
    //  - power()   -> mW
    // Typischerweise werfen diese Methoden keine Exceptions und liefern double.
    const double V  = ina_.voltage();
    const double mA = ina_.current();
    const double mW = ina_.power();

    // Optional einfache Plausibilitätsprüfung:
    if (!std::isfinite(V) || !std::isfinite(mA) || !std::isfinite(mW)) {
        setErr(err, "INA219: ungültige Messwerte");
        return std::nullopt;
    }

    return std::make_tuple(V, mA, mW);
}

void INA219Manager::Shutdown() {
    // regisin/ina219 hält in der Regel keine Ressourcen, daher nur Flag zurücksetzen.
    initialized_ = false;
}
