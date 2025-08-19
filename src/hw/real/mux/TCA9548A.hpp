// hw/mux/TCA9548A.hpp
#pragma once
#include <cstdint>
#include <string>
class I2CBus;

class TCA9548A {
public:
    bool init(I2CBus* bus, uint8_t addr, std::string* err=nullptr);
    bool select(int channel, std::string* err=nullptr);     // 0..7
    bool selectMask(uint8_t mask, std::string* err=nullptr); // 0x00..0xFF
    uint8_t lastMask() const { return last_mask_; }
private:
    I2CBus* bus_ = nullptr;
    uint8_t addr_ = 0x70;
    uint8_t last_mask_ = 0x00;
};
