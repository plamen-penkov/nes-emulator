#include "Cpu6505.hpp"
#include "Bus.hpp"
#include <vector>

Cpu6505::Cpu6505() {
    // Fill the table with 256 blank instructions first to prevent crashes
    lookup = std::vector<Instruction>(256, {"XXX", &Cpu6505::NOP, &Cpu6505::IMP, 2});

    // Overwrite index 0xA9 (169) with LDA Immediate Instruction
    lookup[0xA9] = {"LDA", &Cpu6505::LDA, &Cpu6505::IMM, 2};

    // TODO: Add overwrite for all opcodes
}

Cpu6505::~Cpu6505() {}

void Cpu6505::write(uint16_t addr, uint8_t data) {
    bus->write(addr, data);
}

uint8_t Cpu6505::read(uint16_t addr) {
    return bus->read(addr);
}

void Cpu6505::connectBus(Bus* n) { 
    bus = n; 
}

void Cpu6505::clock() {
    if (cycles == 0) {
        uint8_t opcode = read(programCounter);
        programCounter++;

        // Look up instruction
        Instruction instruction = lookup[opcode];

        // Set the base cycle count for the instruction
        cycles = instruction.cycles;

        // We capture their return values in case they demand an extra clock cycle
        uint8_t extraCycleOne = (this->*(instruction.addrmode))();
        uint8_t extraCycleTwo = (this->*(instruction.operate))();

        if (extraCycleOne && extraCycleTwo) cycles++;

    }
    cycles--;
}

void Cpu6505::reset() {
    accumulator = 0x00;
    x = 0x00;
    y = 0x00;
    stackPointer = 0xFD;
    uint16_t lo = read(0xFFFC);
    uint16_t hi = read(0xFFFD);
    programCounter =  (hi << 8) | lo; // Shift high byte left by 8 bits, then merge with low bits
    status = 0x24; // 0x24 in hex is 00100100 in binary (Bits 5 and 2 are turned on)

    cycles = 8;
}

void Cpu6505::SetFlag(FLAGS6502 f, bool v) {
    if (v) {
        status |= f;
    } else {
        status &= ~f;
    }
}

uint8_t Cpu6505::GetFlag(FLAGS6502 f) {
    return ((status & f) > 0) ? 1 : 0;
}

// --- ADDRESSING MODES ---

// Implied: The instruction doesn't need an address (e.g., NOP)
uint8_t Cpu6505::IMP() {
    return 0;
}

// Immediate: The address is exactly the next byte in memory
uint8_t Cpu6505::IMM() {
    // Set the target address to exactly where we are now
    addr_abs = programCounter;

    // Move the PC past the data so we don't accidentally execute it as a code later!
    programCounter++;

    return 0; // Immediate mode does't require an extra clock
}

// --- OPCODES ---

// NOP: No operation
uint8_t Cpu6505::NOP() {
    return 0;
}

// LDA: Load Accumulator
uint8_t Cpu6505::LDA() {
    // Fetch data from address calculated by the addressing mode
    uint8_t fetchedData = read(addr_abs);
    
    // Load into accumulator
    accumulator = fetchedData;

    // Set the Zero (Z) flag (Bit 1)
    SetFlag(Z, accumulator == 0x00);
    SetFlag(N, (accumulator & 0x80) != 0);

    return 1; // LDA can potentially require an extra clock depending on the address mode
}