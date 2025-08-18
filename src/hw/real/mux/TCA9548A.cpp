#include "hw/mux/TCA9548A.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sstream>

TCA9548A::~TCA9548A() { Close(); }

TCA9548A::TCA9548A(TCA9548A&& other) noexcept {
    fd_ = other.fd_; other.fd_ = -1;
    dev_path_ = std::move(other.dev_path_);
    addr_ = other.addr_;
    last_mask_ = other.last_mask_;
}
TCA9548A& TCA9548A::operator=(TCA9548A&& other) noexcept {
    if (this != &other) {
        Close();
        fd_ = other.fd_; other.fd_ = -1;
        dev_path_ = std::move(other.dev_path_);
        addr_ = other.addr_;
        last_mask_ = other.last_mask_;
    }
    return *this;
}

bool TCA9548A::Init(const std::string& i2c_dev, uint8_t addr, std::string* err) {
    Close();
    dev_path_ = i2c_dev;
    addr_ = addr;

    fd_ = ::open(dev_path_.c_str(), O_RDWR);
    if (fd_ < 0) {
        if (err) { *err = "TCA9548A: open(" + dev_path_ + ") failed: " + std::strerror(errno); }
        return false;
    }
    if (ioctl(fd_, I2C_SLAVE, addr_) < 0) {
        if (err) { *err = "TCA9548A: ioctl(I2C_SLAVE, 0x" + [&]{ std::ostringstream o; o<<std::hex<<(int)addr_; return o.str(); }() + ") failed: " + std::strerror(errno); }
        Close();
        return false;
    }
    // optional: alle Kanäle aus (0x00)
    if (!writeByte(0x00, err)) {
        Close();
        return false;
    }
    return true;
}

void TCA9548A::Close() {
    if (fd_ >= 0) { ::close(fd_); fd_ = -1; }
    last_mask_ = 0x00;
}

bool TCA9548A::writeByte(uint8_t value, std::string* err) {
    if (fd_ < 0) { if (err) *err = "TCA9548A: device not open"; return false; }
    uint8_t buf[1] = { value };
    ssize_t n = ::write(fd_, buf, 1);
    if (n != 1) {
        if (err) { *err = std::string("TCA9548A: write failed: ") + std::strerror(errno); }
        return false;
    }
    last_mask_ = value;
    return true;
}

bool TCA9548A::Select(int channel, std::string* err) {
    if (channel < 0 || channel > 7) {
        if (err) *err = "TCA9548A: channel out of range (0..7)";
        return false;
    }
    const uint8_t mask = static_cast<uint8_t>(1u << channel);
    return SelectMask(mask, err);
}

bool TCA9548A::SelectMask(uint8_t mask, std::string* err) {
    // mask 0x00 = alle aus, 0x01..0x80 je nach Kanal, mehrere Kanäle sind erlaubt
    return writeByte(mask, err);
}
