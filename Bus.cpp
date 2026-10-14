#include "Bus.hpp"

// Constructor
Bus::Bus() {
    // Cllear the RAM contents to zero on startup
    ram.fill(0x00);
}

// Destructor
Bus::~Bus() {}


// Write data to 16-bit address
void Bus::write(uint16_t addr, uint8_t data) {
    // Ensure writing only inside the 64KB ram limit
    if (addr >= 0x0000 && addr <= 0xFFFF) {
        ram[addr] = data;
    }
}

// Read data from 16-bit address
uint8_t Bus::read(uint16_t addr, bool bReadOnly) {
    // Ensure reading only inside the 64KB ram limit
    if (addr >= 0x0000 && addr <= 0xFFFF) {
        return ram[addr];
    }
    return 0x00; // Return zero if out of bounds
}