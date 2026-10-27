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
    uint16_t addr_rel = 0x0000;

    void connectBus(Bus* n);

    void clock();
    void reset();
    
private:
    Bus* bus = nullptr;

    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr);

    void SetFlag(FLAGS6502 f, bool v);
    uint8_t GetFlag(FLAGS6502 f);
    
    bool isAccumulatorMode = false; 

    // ==============================================
    // Addressing Modes (These return an extra cycle penalty if applicable)
    // ==============================================
    uint8_t IMP(); // Implied
    uint8_t IMM(); // Immediate
    uint8_t ZP0(); // Zero Page (An 8-bit address on page 0)
    uint8_t ZPX(); // Zero Page with X offset
    uint8_t ZPY(); // Zero Page with Y offset
    uint8_t REL(); // Relative (Used only for branch instructions, calculates an offset from the Program Counter)
    uint8_t ABS(); // Absolute (A full 16-bit address anywhere in memory)
    uint8_t ABX(); // Absolute with X offset
    uint8_t ABY(); // Absolute with Y offset
    uint8_t IND(); // Indirect (Reads a 16-bit pointer from an Absolute address; used exclusively by the JMP instruction)
    uint8_t IZX(); // Indexed indirect (Reads a 16-bit pointer from a Zero Page address offset by X)
    uint8_t IZY(); // Indexed indirect (Reads a 16-bit pointer from a Zero Page address offset by Y)

    // ==============================================
    // Opcodes (These return an extra cycle penalty if applicable)
    // ==============================================
    // ===== Access Operations =====
    uint8_t STA(); // Store A into memory
    uint8_t STX(); // Store X into memory
    uint8_t STY(); // Story Y into memory
    uint8_t LDA(); // Load Accumulator from memory
    uint8_t LDX(); // Load X from memory
    uint8_t LDY(); // Load Y from memory
    // ===== Transfer Operations =====
    uint8_t TAX(); // Transfer A to X
    uint8_t TXA(); // Transfer X to A
    uint8_t TAY(); // Transfer A to Y
    uint8_t TYA(); // Transfer Y to A
    // ===== Arithmetic Operations =====
    uint8_t ADC(); // Add with carry
    uint8_t SBC(); // Subtract with carry
    uint8_t INC(); // Increment memory location by 1
    uint8_t DEC(); // Decrement memory location by 1
    uint8_t INX(); // Increment X by 1
    uint8_t DEX(); // Decrement X by 1
    uint8_t INY(); // Increment Y by 1
    uint8_t DEY(); // Decrement Y by 1
    // ===== Shift Operations =====
    uint8_t ASL(); // Arithmetic Shift Left
    uint8_t LSR(); // Logical Shift Right
    uint8_t ROL(); // Rotate Left
    uint8_t ROR(); // Rotate Right
    // ===== Bitwise Operations =====
    uint8_t AND(); // AND Accumulator with memory
    uint8_t ORA(); // OR accumulator with memory
    uint8_t EOR(); // XOR accumulator with memory
    uint8_t BIT(); // BIT test
    // ===== Compare Operations =====
    uint8_t CMP(); // Compare A to memory
    uint8_t CPX(); // Compare X to memory
    uint8_t CPY(); // Compare Y to memory
    // ===== Branch Operations =====
    uint8_t BCC(); // Branch if Carry Clear
    uint8_t BCS(); // Branch if Carry Set
    uint8_t BEQ(); // Branch if Equal
    uint8_t BNE(); // Branch if Not Equal
    uint8_t BPL(); // Branch if Plus
    uint8_t BMI(); // Branch if Minus
    uint8_t BVC(); // Branch if Overflow Clear
    uint8_t BVS(); // Branch if Overflow Set
    // ===== Jump Operations =====
    uint8_t JMP(); // Jump the program counter to a location from memory
    uint8_t JSR(); // Jump to Subroutine pushes the current program counter to the stack and then sets the program counter to a new value
    uint8_t RTS(); // Return from Subroutine pulls an address from the stack into the program counter and then increments the program counter
    uint8_t BRK(); // Break triggers an interrupt request (IRQ)
    uint8_t RTI(); // Return from Interrupt
    // ===== Stack Operations =====
    uint8_t PHA(); // Push value of the Accumulator to the stack position
    uint8_t PLA(); // Pull A
    uint8_t PHP(); // Push Processor Status
    uint8_t PLP(); // Pull Processor Status
    uint8_t TXS(); // Transfer X to Stack Pointer
    uint8_t TSX(); // Transfer Stack Pointer to X
    // ===== Flags Operations =====
    uint8_t CLC(); // Clear Carry
    uint8_t SEC(); // Set Carry
    uint8_t CLI(); // Clear Interrupt Disable
    uint8_t SEI(); // Set Interrupt Disable
    uint8_t CLD(); // Clear Decimal
    uint8_t SED(); // Set Decimal
    uint8_t CLV(); // Clear Overflow
    // ===== Other =====
    uint8_t NOP(); // Self explanatory
    uint8_t SLO(); // Shift Left then OR
    uint8_t RLA(); // Rotate Left then AND
    uint8_t SRE(); // Shift Right and XOR
    uint8_t ALR(); // AND then Shift Right
    uint8_t ANC(); // AND with Carry
    uint8_t XXX(); // Illegal
};
