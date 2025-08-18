#pragma once
#include <memory>
#include "hw/IHardware.hpp"
#include "config/ConfigSoftware.hpp"   // ConfigSoftwareView
#include "config/ConfigHardware.hpp"   // falls benötigt (hier ungenutzt im Mock)

std::shared_ptr<sosesta::hw::IHardware> MakeHardware(
    const ConfigSoftwareView& cfg_view,
    const ConfigHardware&     hw_cfg);
