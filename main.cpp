#include "Bus.hpp"
#include "Cpu6505.hpp"
#include <iostream>

int main() {
    Bus nesBus;
    Cpu6505 cpu;
    cpu.connectBus(&nesBus);

    // 1. Inject tiny program into RAM at address 0x8000
    // 0xA9 is LDA Immediate, 0x42 is the data we want to load
    nesBus.write(0x8000, 0xA9);
    nesBus.write(0x8001, 0x42);

    // 2. Set the reset vector at 0xFFFC to point to out program at 0x8000
    nesBus.write(0xFFFC, 0x00);
    nesBus.write(0xFFFD, 0x80);

    // 3. Boot the CPU (This takes 8 cycles and sets the program counter to 0x8000)
    cpu.reset();
    
    // Tick the clock 8 times to finish the reset process
    for (int i = 0; i < 8; i++) cpu.clock();

    // 4. Execute the LDA instruction (LDA Immediate takes 2 cycles)
    cpu.clock();
    cpu.clock();

    // 5. Print the result
    std::cout << "Accumulator: 0x" << std::hex << (int)cpu.accumulator << std::endl;

    if (cpu.accumulator == 0x42) {
        std::cout << "SUCCESS! Your CPU executed its first instruction!" << std::endl;
    } else {
        std::cout << "FAILED. Accumulator does not match." << std::endl;
    }
    return 0;
}