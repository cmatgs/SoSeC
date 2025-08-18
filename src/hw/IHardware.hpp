// src/hw/IHardware.hpp
#pragma once
#include <vector>
#include "app/data/SensorData.hpp"

namespace sosesta::hw
{

/**
 * @brief Schnittstelle für alle Hardware-Implementierungen.
 *
 * Diese Schnittstelle ist plattformunabhängig und kennt keine Details
 * zu konkreten Config-Strukturen. Implementierungen (Mock oder Real)
 * können im Konstruktor beliebige Parameter entgegennehmen.
 */
class IHardware
{
public:
    virtual ~IHardware() = default;

    /// Initialisiert die Hardware (z. B. I2C, GPIO, DAQ)
    virtual void Initialize() = 0;

    /// Fährt die Hardware sauber herunter
    virtual void Shutdown() = 0;

    /// Liest alle Sensordaten in den übergebenen Vektor
    virtual void UpdateSensors(std::vector<SensorData>& sensors) = 0;

    /// Schaltet ein bestimmtes Relais ein/aus
    virtual void ToggleRelay(int channel, bool state) = 0;

    /// Schaltet alle Relais ein
    virtual void TurnAllRelaysOn() = 0;

    /// Schaltet alle Relais aus
    virtual void TurnAllRelaysOff() = 0;
};

} // namespace sosesta::hw
