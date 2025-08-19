// hw/ina/MuxedIna219.hpp
#pragma once
#include <string>
struct InaReading { float bus_V=0, current_mA=0, power_mW=0; };

class TCA9548A; class INA219;

class MuxedIna219 {
public:
    MuxedIna219(TCA9548A& mux, INA219& ina): mux_(mux), ina_(ina) {}
    bool read(int channel, InaReading& out, std::string* err=nullptr);
private:
    TCA9548A& mux_; INA219& ina_;
};
