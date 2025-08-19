// i2c/I2CBus.hpp
#pragma once
#include <string>
#include <cstdint>
#include <vector>

class I2CBus {
public:
    I2CBus() = default;
    ~I2CBus();

    bool openDev(const std::string& dev, std::string* err=nullptr);
    void closeDev();

    bool setSlave(uint8_t addr, std::string* err=nullptr);

    // simple primitives
    bool writeBytes(const uint8_t* data, size_t len, std::string* err=nullptr);
    bool readBytes(uint8_t* data, size_t len, std::string* err=nullptr);

    // helper: write register, read 16-bit big-endian
    bool writeReg(uint8_t reg, std::string* err=nullptr);
    bool readReg16BE(uint8_t reg, uint16_t* out, std::string* err=nullptr);
    bool writeReg16BE(uint8_t reg, uint16_t val, std::string* err=nullptr);

private:
    int fd_ = -1;
    std::string path_;
};
