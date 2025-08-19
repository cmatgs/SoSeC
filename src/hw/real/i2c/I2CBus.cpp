// i2c/I2CBus.cpp
#include "I2CBus.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <cerrno>
#include <cstring>

I2CBus::~I2CBus(){ closeDev(); }

bool I2CBus::openDev(const std::string& dev, std::string* err){
    closeDev();
    fd_ = ::open(dev.c_str(), O_RDWR);
    if (fd_ < 0) { if(err)*err="open("+dev+"):"+std::strerror(errno); return false; }
    path_ = dev; return true;
}

void I2CBus::closeDev(){ if(fd_>=0){::close(fd_); fd_=-1;} }

bool I2CBus::setSlave(uint8_t addr, std::string* err){
    if(fd_<0){ if(err)*err="I2CBus not open"; return false; }
    if (ioctl(fd_, I2C_SLAVE, addr) < 0){ if(err)*err="ioctl(I2C_SLAVE):"+std::string(std::strerror(errno)); return false; }
    return true;
}

bool I2CBus::writeBytes(const uint8_t* d, size_t n, std::string* err){
    if(fd_<0){ if(err)*err="I2CBus not open"; return false; }
    ssize_t w = ::write(fd_, d, n);
    if (w != (ssize_t)n){ if(err)*err="write:"+std::string(std::strerror(errno)); return false; }
    return true;
}

bool I2CBus::readBytes(uint8_t* d, size_t n, std::string* err){
    if(fd_<0){ if(err)*err="I2CBus not open"; return false; }
    ssize_t r = ::read(fd_, d, n);
    if (r != (ssize_t)n){ if(err)*err="read:"+std::string(std::strerror(errno)); return false; }
    return true;
}

bool I2CBus::writeReg(uint8_t reg, std::string* err){
    return writeBytes(&reg, 1, err);
}

bool I2CBus::readReg16BE(uint8_t reg, uint16_t* out, std::string* err){
    if(!writeReg(reg, err)) return false;
    uint8_t b[2];
    if(!readBytes(b,2,err)) return false;
    *out = (uint16_t(b[0])<<8)|b[1];
    return true;
}

bool I2CBus::writeReg16BE(uint8_t reg, uint16_t val, std::string* err){
    uint8_t b[3] = {reg, uint8_t(val>>8), uint8_t(val&0xFF)};
    return writeBytes(b,3,err);
}
