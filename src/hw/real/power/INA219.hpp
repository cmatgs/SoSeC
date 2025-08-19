// hw/ina/INA219.hpp
#pragma once
#include <cstdint>
#include <string>

class I2CBus;

enum InaRange { RANGE_16V=0, RANGE_32V=1 };
enum InaGain  { GAIN_1_40MV=0, GAIN_2_80MV, GAIN_4_160MV, GAIN_8_320MV };
enum InaAdc   { ADC_9BIT=0, ADC_10BIT, ADC_11BIT, ADC_12BIT=3,
                ADC_2SAMP=9, ADC_4SAMP, ADC_8SAMP, ADC_16SAMP, ADC_32SAMP, ADC_64SAMP, ADC_128SAMP };

class INA219 {
public:
    bool init(I2CBus* bus, uint8_t addr, float shunt_ohms, float max_expected_amps, std::string* err=nullptr);
    bool configure(InaRange vr, InaGain gain, InaAdc bus_adc, InaAdc shunt_adc, std::string* err=nullptr);

    bool reset(std::string* err=nullptr);
    bool sleep(std::string* err=nullptr);
    bool wake(std::string* err=nullptr);

    bool voltage(float* V, std::string* err=nullptr);       // V
    bool shuntVoltage(float* mV, std::string* err=nullptr); // mV
    bool current(float* mA, std::string* err=nullptr);      // mA
    bool power(float* mW, std::string* err=nullptr);        // mW

private:
    bool writeReg(uint8_t reg, uint16_t val, std::string* err);
    bool readReg(uint8_t reg, uint16_t* val, std::string* err);
    void computeCalibration(float shunt_volts_max, std::string* err);
private:
    I2CBus* bus_ = nullptr; uint8_t addr_ = 0x40;
    float shunt_ohms_ = 0.1f;
    float max_expected_amps_ = 2.0f;
    float min_device_current_lsb_ = 0.0f;
    float current_lsb_ = 0.0f; float power_lsb_ = 0.0f;
    InaRange vrange_ = RANGE_32V; InaGain gain_ = GAIN_8_320MV;
};
