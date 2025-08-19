// hw/ina/MuxedIna219.cpp
#include "MuxedIna219.hpp"
#include "hw/mux/TCA9548A.hpp"
#include "hw/ina/INA219.hpp"

bool MuxedIna219::read(int ch, InaReading& out, std::string* err){
    if(!mux_.select(ch, err)) return false;
    
    float V=0,I=0,P=0;
    if(!ina_.voltage(&V,err)) return false;
    if(!ina_.current(&I,err)) return false;
    if(!ina_.power(&P,err)) return false;
    out.bus_V=V; out.current_mA=I; out.power_mW=P;
    return true;
}
