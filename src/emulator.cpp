/*
   Copyright 2023 Eduardo Antunes S. Vieira <eduardoantunes986@gmail.com>

   This file is part of libre-nes.

   libre-nes is free software: you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation, either version 3 of the License, or (at your option) any later
   version.

   libre-nes is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with
   libre-nes. If not, see <https://www.gnu.org/licenses/>.
*/

#include <cstdint>
#include <iostream>
#include "emulator.hpp"

using namespace nes;

void Emulator::load_prog(const std::vector<uint8_t> &prog, uint16_t inst_nr) {
    instruction_nr = inst_nr;
    uint16_t addr = prog_start;
    for(auto byte : prog) {
        ram[addr++] = byte;
    }
}

void Emulator::start() {
    std::cout << "Initial state of the registers:\n";
    cpu.show_registers();
    for(uint16_t i = 0; i < instruction_nr; ++i) {
        cpu.show_opcode();
        cpu.single_step();
        std::cout << "CPU registers after execution:\n";
        cpu.show_registers();
        std::cout << "Stack: ";
        cpu.show_stack();
        std::cout << '\n';
    }
}

uint8_t Emulator::read(uint16_t addr) const {
    if(addr >= 0x0000 && addr <= 0x1FFF)
        // RAM is mirrored throught this range
        return ram[addr & 0x07FF];
    else if(addr == 0xFFFC)
        // This address must contain the low byte of the program start
        return (prog_start & 0x00FF);
    else if(addr == 0xFFFD)
        // This address must contain the high byte of the program start
        return (prog_start & 0xFF00) >> 8;
    return 0;
}

void Emulator::write(uint16_t addr, uint8_t data) {
    if(addr >= 0x0000 && addr <= 0x1FFF)
        // RAM is mirrored throught this range
        ram[addr & 0x07FF] = data;
}
