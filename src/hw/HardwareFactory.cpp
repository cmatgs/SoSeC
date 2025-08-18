#include "hw/HardwareFactory.hpp"

#if defined(USE_MOCK)
  #include "hw/mock/MockHardware.hpp"
#else
  #include "hw/real/RealHardware.hpp" 
#endif

std::shared_ptr<sosesta::hw::IHardware> MakeHardware(
    const ConfigSoftwareView& cfg_view,
    const ConfigHardware&     /*hw_cfg*/)
{
#if defined(USE_MOCK)
    return std::make_shared<sosesta::hw::MockHardware>(cfg_view);
#else
    return std::make_shared<sosesta::hw::RealHardware>(/* cfg_view, hw_cfg */);
#endif
}
