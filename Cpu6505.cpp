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

// ==============================================
// ADDRESSING MODES
// ==============================================

// Implicid: The instruction doesn't need an address (e.g., NOP)
uint8_t Cpu6505::IMP() {
    return 0;
}

// Immediate: The address is exactly the next byte in memory
uint8_t Cpu6505::IMM() {
    // Set the target address to exactly where we are now
    addr_abs = programCounter;
    programCounter++;

    return 0; // Immediate mode does't require an extra clock
}

// Zero Page 0
uint8_t Cpu6505::ZP0() {
    // Read the exact Zero Page memory address provided by the instruction
    addr_abs = read(programCounter);
    programCounter++;

    // Bitwise AND with 0x00FF strictly enforces that the high byte is 0x00
    addr_abs &= 0x00FF;

    return 0;
}

// Zero Page with X offset
uint8_t Cpu6505::ZPX() {
    // Read the exact Zero Page memory address provided by the instruction
    uint8_t base_addr = read(programCounter);
    programCounter++;

    // Bitwise AND with the provided instruction with X offset
    addr_abs = (base_addr + x) & 0x00FF;

    return 0;
}

// Zero Page with Y offset
uint8_t Cpu6505::ZPY() {
    // Read the exact Zero Page memory address provided by the instruction
    uint8_t base_addr = read(programCounter);
    programCounter++;

    // Bitwise AND with the provided instruction with Y offset
    addr_abs = (base_addr + y) & 0x00FF;

    return 0;
}

// Relative
uint8_t Cpu6505::REL() {
    // Read the 8-bit offset
    addr_rel = read(programCounter);
    programCounter++;

    // Sign Extension: If the 7th bit is 1, the number is negative.
    // We force the high byte to be 0xFF (e.g., 0x0080 becomes 0xFF80).
    if (addr_rel & 0x80) {
        addr_rel |= 0xFF00;
    }
    
    return 0; // Branching cycle penalties are handled by the opcode itself, not the addressing mode
}

// Absolute 
uint8_t Cpu6505::ABS() {
    uint8_t lo = read(programCounter);
    programCounter++;
    uint8_t hi = read(programCounter);
    programCounter++;

    addr_abs = (hi << 8) | lo;

    return 0;
}

// Absolute with X offset
uint8_t Cpu6505::ABX() {
    uint8_t lo = read(programCounter);
    programCounter++;
    uint8_t hi = read(programCounter);
    programCounter++;

    uint8_t base_addr = (hi << 8) | lo;
    addr_abs = base_addr + x;

    // Check if adding X caused the high byte (the page) to change
    if ((addr_abs & 0xFF00) != (hi << 8)) {
        return 1;
    }
    return 0;
}

// Absolute with Y offset
uint8_t Cpu6505::ABY() {
    uint8_t lo = read(programCounter);
    programCounter++;
    uint8_t hi = read(programCounter);
    programCounter++;

    uint8_t base_addr = (hi << 8) | lo;
    addr_abs = base_addr + y;

    // Check if adding Y caused the high byte (the page) to change
    if ((addr_abs & 0xFF00) != (hi << 8)) {
        return 1;
    }
    return 0;
}

// Indirect
uint8_t Cpu6505::IND() {
    uint16_t ptr_lo = read(programCounter);
    programCounter++;
    uint16_t ptr_hi = read(programCounter);
    programCounter++;

    uint16_t ptr = (ptr_hi << 8) | ptr_lo;
    // Simulate the page boundary hardware bug
    if (ptr_lo == 0x00FF) {
        // Bug: Read high byte from the start of the current page instead of the next page
        addr_abs = (read(ptr & 0xFF00) << 8) | read(ptr);
    } else {
        // Normal behavior
        addr_abs = (read(ptr + 1) << 8) | read(ptr);
    }

    return 0;
}

// Indexed Indirect with X offset
uint8_t Cpu6505::IZX() {
    uint16_t t = read(programCounter);
    programCounter++;

    // Read the 16-bit base address from the Zero Page
    uint16_t lo = read(t & 0x00FF);
    uint16_t hi = read((t + 1) & 0x00FF);

    uint16_t base_addr = (hi << 8) | lo;
    
    // Add X to find the final target address
    addr_abs = base_addr + x;

    // Check if adding x caused a page cross
    if ((addr_abs & 0xFF00) != (hi << 8)) {
        return 1;
    }
    return 0;
}

// Indexed Indirect with Y offset
uint8_t Cpu6505::IZY() {
    uint16_t t = read(programCounter);
    programCounter++;

    // Read the 16-bit base address from the Zero Page
    uint16_t lo = read(t & 0x00FF);
    uint16_t hi = read((t + 1) & 0x00FF);

    uint16_t base_addr = (hi << 8) | lo;
    
    // Add Y to find the final target address
    addr_abs = base_addr + y;

    // Check if adding Y caused a page cross
    if ((addr_abs & 0xFF00) != (hi << 8)) {
        return 1;
    }
    return 0;
}

// ==============================================
// OPCODES 
// ==============================================

// ===== ACCESS =====
uint8_t Cpu6505::STA() {
    write(addr_abs, accumulator);
    return 0;
}

uint8_t Cpu6505::STX() {
    write(addr_abs, x);
    return 0;
}

uint8_t Cpu6505::STY() {
    write(addr_abs, y);
    return 0;
}

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

uint8_t Cpu6505::LDX() {
    // Fetch data from address calculated by the addressing mode
    uint8_t fetchedData = read(addr_abs);
    
    // Load into x
    x = fetchedData;

    // Set the Zero (Z) flag (Bit 1)
    SetFlag(Z, x == 0x00);
    SetFlag(N, (x & 0x80) != 0);

    return 1; // LDA can potentially require an extra clock depending on the address mode
}

uint8_t Cpu6505::LDY() {
    // Fetch data from address calculated by the addressing mode
    uint8_t fetchedData = read(addr_abs);
    
    // Load into y
    y = fetchedData;

    // Set the Zero (Z) flag (Bit 1)
    SetFlag(Z, y == 0x00);
    SetFlag(N, (y & 0x80) != 0);

    return 1; // LDA can potentially require an extra clock depending on the address mode
}

// ===== TRANSFER =====
uint8_t Cpu6505::TAX() {
    x = accumulator;
    
    SetFlag(Z, x == 0x00);
    SetFlag(N, (x &0x80) != 0);

    return 0;
}

uint8_t Cpu6505::TXA() {
    accumulator = x;
    
    SetFlag(Z, accumulator == 0x00);
    SetFlag(N, (accumulator & 0x80) != 0);

    return 0;
}

uint8_t Cpu6505::TAY() {
    y = accumulator;
    
    SetFlag(Z, y == 0x00);
    SetFlag(N, (y &0x80) != 0);

    return 0;
}

uint8_t Cpu6505::TYA() {
    accumulator = y;
    
    SetFlag(Z, accumulator == 0x00);
    SetFlag(N, (accumulator & 0x80) != 0);

    return 0;
}

// ===== ARITHMETIC =====
uint8_t Cpu6505::ADC() {
    uint8_t fetchedData = read(addr_abs);
    uint16_t temp = uint16_t(accumulator + fetchedData + GetFlag(C));

    SetFlag(C, temp > 255);
    SetFlag(Z, (temp & 0x00FF) == 0);
    SetFlag(N, temp & 0x80);
    SetFlag(V, (~((uint16_t)accumulator ^ (uint16_t)fetchedData) & ((uint16_t)accumulator ^ (uint16_t)temp)) & 0x0080);
    
    accumulator = temp;
    return 1;
}

uint8_t Cpu6505::SBC() {
    uint8_t fetchedData = read(addr_abs);
    uint16_t invertedData = ((uint16_t)fetchedData) ^ 0x00FF;
    uint16_t temp = (uint16_t)accumulator + invertedData + (uint16_t)GetFlag(C);

    SetFlag(C, temp > 255);
    SetFlag(Z, (temp & 0x00FF) == 0);
    SetFlag(N, temp & 0x80);
    SetFlag(V, (~((uint16_t)accumulator ^ invertedData) & ((uint16_t)accumulator ^ (uint16_t)temp)) & 0x0080);

    accumulator = temp;
    return 1;
}

uint8_t Cpu6505::INC() {
    uint8_t fetchedData = read(addr_abs);
    fetchedData++;
    write(addr_abs, fetchedData);

    SetFlag(Z, fetchedData == 0x00);
    SetFlag(N, fetchedData & 0x80);

    return 1;
}

uint8_t Cpu6505::DEC() {
    uint8_t fetchedData = read(addr_abs);
    fetchedData--;
    write(addr_abs, fetchedData);

    SetFlag(Z, fetchedData == 0x00);
    SetFlag(N, fetchedData & 0x80);

    return 1;
}

uint8_t Cpu6505::INX() {
    x++;

    SetFlag(Z, x == 0x00);
    SetFlag(N, x & 0x80);

    return 1;
}

uint8_t Cpu6505::DEX() {
    x--;

    SetFlag(Z, x == 0x00);
    SetFlag(N, x & 0x80);

    return 1;
}

uint8_t Cpu6505::INY() {
    y++;

    SetFlag(Z, y == 0x00);
    SetFlag(N, y & 0x80);

    return 1;
}

uint8_t Cpu6505::DEY() {
    y--;

    SetFlag(Z, y == 0x00);
    SetFlag(N, y & 0x80);

    return 1;
}

// ===== SHIFT =====

// ===== BITWISE =====
uint8_t Cpu6505::AND() {
    uint8_t fetchedData = read(addr_abs);
    accumulator = accumulator & fetchedData;

    SetFlag(Z, y == 0x00);
    SetFlag(N, (y & 0x80) != 0);
    return 1;
}

uint8_t Cpu6505::ORA() {
    uint8_t fetchedData = read(addr_abs);
    accumulator = accumulator | fetchedData;

    SetFlag(Z, y == 0x00);
    SetFlag(N, (y & 0x80) != 0);
    return 1;
}

uint8_t Cpu6505::EOR() {
    uint8_t fetchedData = read(addr_abs);
    accumulator = accumulator ^ fetchedData;

    SetFlag(Z, y == 0x00);
    SetFlag(N, (y & 0x80) != 0);
    return 1;
}

uint8_t Cpu6505::BIT() {
    uint8_t fetchedData = read(addr_abs);
    uint8_t temp = accumulator & fetchedData;
    
    SetFlag(Z, temp == 0x00);
    SetFlag(N, (fetchedData & (1 << 7)) != 0);
    SetFlag(V, (fetchedData & (1 << 6)) != 0);
    
    return 0; // BIT never triggers an extra cycle penalty
}

// ===== COMPARE =====
uint8_t Cpu6505::CMP() {
    uint8_t fetchedData = read(addr_abs);
    uint8_t result = accumulator - fetchedData;

    SetFlag(C, accumulator >= fetchedData);
    SetFlag(Z, result == 0x00);
    SetFlag(N, (result & 0x80) != 0);

    return 1;
}

uint8_t Cpu6505::CPX() {
    uint8_t fetchedData = read(addr_abs);
    uint8_t result = x - fetchedData;

    SetFlag(C, x >= fetchedData);
    SetFlag(Z, result == 0x00);
    SetFlag(N, (result & 0x80) != 0);

    return 1;
}

uint8_t Cpu6505::CPY() {
    uint8_t fetchedData = read(addr_abs);
    uint8_t result = y - fetchedData;

    SetFlag(C, y >= fetchedData);
    SetFlag(Z, result == 0x00);
    SetFlag(N, (result & 0x80) != 0);

    return 1;
}

// ===== BRANCH =====

// ===== JUMP =====
uint8_t Cpu6505::JMP() {
    programCounter = read(addr_abs);
    return 1;
}

uint8_t Cpu6505::JSR() {
    // Calculate the return address (next instruction address - 1)
    uint16_t return_addr = programCounter - 1;
    
    // Push the High Byte to the stack first (6502 pushes high then low for 16-bit addresses)
    write(0x0100 + stackPointer, (return_addr << 8) & 0x00FF);
    stackPointer--;

    // Push the Low Byte to the stack
    write(0x0100 + stackPointer, return_addr & 0x00FF);
    stackPointer--;

    // Set the Program Counter to the target subroutine address calculated by ABS()
    programCounter = addr_abs;

    return 0;
}

uint8_t Cpu6505::RTS() {
    stackPointer++;
    uint8_t lo = read(0x0100 + stackPointer);
    stackPointer++;
    uint8_t hi = read(0x0100 + stackPointer);

    uint16_t return_addr = (hi << 8) | lo;
    programCounter = return_addr + 1;

    return 0;
}

uint8_t Cpu6505::BRK() {
    // Advance the Program Counter 
    // (BRK leaves a 1-byte gap before pushing the return address)
    programCounter++;

    // Push Program Counter to the stack (High byte first)
    write(0x0100 + stackPointer, (programCounter << 8) & 0x00FF);
    stackPointer--;
    write(0x0100 + stackPointer, programCounter & 0x00FF);
    stackPointer--;

    // Push Status Register to the stack with Break (Bit 4) and Unused (Bit 5) set to 1
    // (Assuming your status register is a uint8_t named 'status')
    write(0x0100 + stackPointer, status | (1 << 4) | (1 << 5));
    stackPointer--;

    // Set the Interupt Disable (I) flag
    SetFlag(I, true);

    // Read the IRQ/BRK vector from 0xFFFE and 0xFFFF to find the new Program Counter
    uint16_t lo = read(0xFFFE);
    uint16_t hi = read(0xFFFF);
    programCounter = (hi << 8) | lo;

    return 0; // BRK cycle count is fixed
}

uint8_t Cpu6505::RTI() {
    stackPointer++;
    status = read(0x0100 + stackPointer);

    // The physical 6502 ignores bits 4 (Break) and forces bit 5 (Unused) to 1 
    // when restoring the status register from the stack.
    status &= ~(1 << 4); 
    status |= (1 << 5);
    
    stackPointer++;
    uint8_t lo = read(0x0100 + stackPointer);
    stackPointer++;
    uint8_t hi = read(0x0100 + stackPointer);

    programCounter = (hi << 8) | lo;;
    
    return 0;
}

// ===== STACK =====
uint8_t Cpu6505::PHA() {
    write((0x0100 + stackPointer), accumulator);
    stackPointer--;
    
    return 1;
}

uint8_t Cpu6505::PLA() {
    stackPointer++;
    accumulator = read(0x0100 + stackPointer);
    
    SetFlag(Z, accumulator == 0x00);
    SetFlag(N, (accumulator & 0x80) != 0);
    
    return 1;
}

uint8_t Cpu6505::PHP() {
    // Push status with Break and Unused bits set to 1
    write(0x0100 + stackPointer, status | (1 << 4) | (1 << 5));
    stackPointer--;

    return 1;
}

uint8_t Cpu6505::PLP() {
    stackPointer++;
    status = read(0x0100 + stackPointer);

    status &= ~(1 << 4); 
    status |= (1 << 5);

    return 1;
}

uint8_t Cpu6505::TXS() {
    stackPointer = x;
    return 1;
}

uint8_t Cpu6505::TSX() {
    x = stackPointer;

    SetFlag(Z, x == 0x00);
    SetFlag(N, (x & 0x80) != 0);

    return 1;
}

// ===== FLAGS =====

// ===== OTHER =====

uint8_t Cpu6505::NOP() {
    return 0;
}
