#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Instruction.h"

// Forward declaration of Bus so the CPU knows it exists
class Bus;

class Cpu6505
{    
public:
    Cpu6505();
    ~Cpu6505();

    enum FLAGS6502 {
        C = (1 << 0), // Carry Bit
        Z = (1 << 1), // Zero
        I = (1 << 2), // Disable Interrupts
        D = (1 << 3), // Decimal Mode (unused)
        B = (1 << 4), // Break
        U = (1 << 5), // Unused
        V = (1 << 6), // Overflow
        N = (1 << 7)  // Negative
    };
    
    // Registers
    uint8_t accumulator     = 0x00;
    uint8_t x               = 0x00;
    uint8_t y               = 0x00;
    uint8_t stackPointer    = 0x00;
    uint16_t programCounter = 0x0000;
    uint8_t status          = 0x00;
    uint8_t cycles          = 0;
    std::vector<Instruction> lookup;    

    uint16_t addr_abs = 0x0000;

    void connectBus(Bus* n);

    void clock();
    void reset();
    
private:
    Bus* bus = nullptr;

    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr);

    void SetFlag(FLAGS6502 f, bool v);
    uint8_t GetFlag(FLAGS6502 f);

    // Addressing Modes (These return an extra cycle penalty if applicable)
    uint8_t IMP(); // Implied
    uint8_t IMM(); // Immediate

    // Opcodes (These return an extra cycle penalty if applicable)
    uint8_t NOP(); // No Operation
    uint8_t LDA(); // Load Accumulator
};
