#pragma once
#include <cstdint>
#include <array>

class Bus
{
public:
    Bus();
    ~Bus();

    // 64KB of address space(from 0x0000 to 0xFFFF)
    std::array<uint8_t, 64*1024> ram;

    // CPU calls these to interact with the system
    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr, bool bReadOnly = false);
};
