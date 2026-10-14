#pragma once
#include <string>
#include <cstdint>

class Cpu6505;

// The blueprint for a single entry in the lookup table
struct Instruction {
    std::string name; // The human-readable name (e.g. "LDA")

    // A pointer to a function inside Cpu6505 that takes no arguments and returns a uint8_t
    uint8_t (Cpu6505::*operate)() = nullptr;

    // A pointer to a function inside Cpu6505 for the addressing mode
    uint8_t (Cpu6505::*addrmode)() = nullptr;

    // Base cycle count for this specific opcode
    uint8_t cycles = 0;
};