#include "Bus.hpp"
#include "Cpu6505.hpp"
#include <iostream>
#include <stdio.h>

int main() {
    Bus nesBus;
    Cpu6505 cpu;
    cpu.connectBus(&nesBus);

    cpu.LoadRom("nestest.nes");

    cpu.reset();

    cpu.programCounter = 0xC000; // Temporary for the NESTEST
    
    while (true) {
        if (cpu.cycles == 0) cpu.LogState();
        cpu.clock();

        if (cpu.programCounter == 0xC66E) break;
    }
    
    return 0;
}