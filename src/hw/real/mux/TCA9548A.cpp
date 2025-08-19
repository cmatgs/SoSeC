// hw/mux/TCA9548A.cpp
#include "TCA9548A.hpp"
#include "i2c/I2CBus.hpp"

bool TCA9548A::init(I2CBus* bus, uint8_t addr, std::string* err){
    bus_ = bus; addr_ = addr; last_mask_ = 0;
    if(!bus_){ if(err)*err="TCA9548A: null bus"; return false; }
    if(!bus_->setSlave(addr_, err)) return false;
    // alle Kanäle aus
    if(!selectMask(0x00, err)) return false;
    return true;
}
bool TCA9548A::select(int ch, std::string* err){
    if (ch < 0 || ch > 7){ if(err)*err="TCA9548A: channel out of range"; return false; }
    return selectMask(uint8_t(1u<<ch), err);
}
bool TCA9548A::selectMask(uint8_t mask, std::string* err){
    if(!bus_){ if(err)*err="TCA9548A: uninitialized"; return false; }
    if(!bus_->setSlave(addr_, err)) return false;
    uint8_t b[1] = { mask };
    if(!bus_->writeBytes(b,1,err)) return false;
    last_mask_ = mask; return true;
}
