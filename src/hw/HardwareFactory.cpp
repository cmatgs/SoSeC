#include "hw/HardwareFactory.hpp"

#ifdef USE_MOCK
  #include "hw/mock/MockHardware.hpp"
#else
  #include "hw/RealHardware.hpp"
#endif

std::unique_ptr<IHardware> MakeHardware(const AppConfigView& cfg) {
#ifdef USE_MOCK
    return std::make_unique<MockHardware>(cfg);               // ggf. Mock-Options hier
#else
    return std::make_unique<RealHardware>(cfg);
#endif
}
    