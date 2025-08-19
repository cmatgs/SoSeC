// hw/ina/INA219.cpp
#include "INA219.hpp"
#include "i2c/I2CBus.hpp"
#include <cmath>

// Register & consts wie in deiner Version:
static constexpr uint8_t REG_CONFIG=0x00, REG_SHUNT=0x01, REG_BUS=0x02, REG_POWER=0x03, REG_CURRENT=0x04, REG_CAL=0x05;
static constexpr float SHUNT_mV_LSB=0.01f; // 10uV
static constexpr float BUS_mV_LSB=4.0f;    // 4mV
static constexpr float CAL_FACTOR=0.04096f;
static constexpr uint16_t MAX_CAL=0xFFFE;
static constexpr int BRNG=13, PG0=11, BADC1=7, SADC1=3;
static constexpr uint16_t CONT_SH_BUS = 7;

bool INA219::init(I2CBus* bus, uint8_t addr, float shunt, float maxA, std::string* err){
    bus_ = bus; addr_ = addr; shunt_ohms_ = shunt; max_expected_amps_ = maxA;
    if(!bus_){ if(err)*err="INA219: null bus"; return false; }
    min_device_current_lsb_ = CAL_FACTOR / (shunt_ohms_ * MAX_CAL);
    return true;
}

bool INA219::writeReg(uint8_t reg, uint16_t val, std::string* err){
    if(!bus_){ if(err)*err="INA219: uninitialized"; return false; }
    if(!bus_->setSlave(addr_, err)) return false;
    return bus_->writeReg16BE(reg, val, err);
}

bool INA219::readReg(uint8_t reg, uint16_t* out, std::string* err){
    if(!bus_){ if(err)*err="INA219: uninitialized"; return false; }
    if(!bus_->setSlave(addr_, err)) return false;
    return bus_->readReg16BE(reg, out, err);
}

bool INA219::reset(std::string* err){ return writeReg(REG_CONFIG, (1u<<15), err); }

bool INA219::sleep(std::string* err){
    uint16_t cfg; if(!readReg(REG_CONFIG,&cfg,err)) return false;
    return writeReg(REG_CONFIG, cfg & 0xFFF8, err);
}

bool INA219::wake(std::string* err){
    uint16_t cfg; if(!readReg(REG_CONFIG,&cfg,err)) return false;
    if(!writeReg(REG_CONFIG, cfg | 0x0007, err)) return false;
    usleep(40); return true;
}

void INA219::computeCalibration(float shunt_volts_max, std::string*){
    const float max_possible_amps = shunt_volts_max / shunt_ohms_;
    float current_lsb = (max_expected_amps_ < max_possible_amps)
        ? (max_expected_amps_ / 32770.0f)  // siehe deine Version: 32770 statt 32767
        : (max_possible_amps / 32770.0f);
    if (current_lsb < min_device_current_lsb_) current_lsb = min_device_current_lsb_;
    current_lsb_ = current_lsb;
    power_lsb_   = current_lsb_ * 20.0f;
}

bool INA219::configure(InaRange vr, InaGain gain, InaAdc badc, InaAdc sadc, std::string* err){
    vrange_ = vr; gain_ = gain;
    const float GAIN_VOLTS[4] = {0.04f,0.08f,0.16f,0.32f};
    const int   BUS_RANGE[2]  = {16,32};

    if(!reset(err)) return false;

    computeCalibration(GAIN_VOLTS[gain_], err);
    const uint16_t calib = (uint16_t) std::trunc(CAL_FACTOR / (current_lsb_ * shunt_ohms_));
    if(!writeReg(REG_CAL, calib, err)) return false;

    const uint16_t cfg = (uint16_t)((vr<<BRNG) | (gain<<PG0) | (badc<<BADC1) | (sadc<<SADC1) | CONT_SH_BUS);
    return writeReg(REG_CONFIG, cfg, err);
}

bool INA219::voltage(float* V, std::string* err){
    uint16_t v; if(!readReg(REG_BUS,&v,err)) return false;
    v >>= 3; *V = (float)v * BUS_mV_LSB / 1000.0f; return true;
}

bool INA219::shuntVoltage(float* mV, std::string* err){
    uint16_t r; if(!readReg(REG_SHUNT,&r,err)) return false;
    *mV = SHUNT_mV_LSB * (int16_t)r; return true;
}

bool INA219::current(float* mA, std::string* err){
    uint16_t r; if(!readReg(REG_CURRENT,&r,err)) return false;
    int16_t s = (int16_t)r;
    if (s > 32767) s -= 65536; // deine Version macht dieses Wrap-Handling ebenso 
    *mA = s * current_lsb_ * 1000.0f; return true;
}

bool INA219::power(float* mW, std::string* err){
    uint16_t r; if(!readReg(REG_POWER,&r,err)) return false;
    int16_t s = (int16_t)r;
    *mW = s * power_lsb_ * 1000.0f; return true;
}
