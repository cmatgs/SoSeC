#pragma once
#include "hw/IHardware.hpp"
#include <memory>

// Liefert je nach Compile-Time-Schalter USE_MOCK eine Real- oder Mock-Implementierung.
std::unique_ptr<IHardware> MakeHardware(const AppConfigView& cfg);
