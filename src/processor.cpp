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
#include "processor.hpp"

using namespace nes;

uint8_t Processor::get_flag(Flag flag) const {
    uint8_t f = static_cast<uint8_t>(flag);
    return status & f;
}

void Processor::set_flag(Flag flag, bool state) {
    uint8_t f = static_cast<uint8_t>(flag);
    if(state)
        status |= f;
    else
        status &= ~f;
}

// TODO account for all of the complexity that's missing

uint16_t Processor::get_address() {
    // Get an absolute address based on the addressing mode
    uint16_t ptr, addr;
    switch(addr_mode) {
        case Addressing::Null:
            // Addressing mode wasn't properly initialized
            std::cerr << "Invalid addressing mode!\n";
            return 0;
        case Addressing::Implicit:
            // No absolute address to fetch
            return 0;
        case Addressing::Accumulator:
            // No absolute address to fetch
            return 0;
        case Addressing::Immediate:
            // No absolute address to fetch (here, we could also return the
            // address of the following byte, but this is fine for now)
            return 0;

        case Addressing::ZeroPage:
            // A zero page address is stored in the next byte
            addr = bus.read(pc++);
            return addr & 0x00FF;
        case Addressing::ZeroPage_x:
            // The zero page address in the next byte is summed with the
            // contents of the x register.
            addr = bus.read(pc++);
            return (addr + x) & 0x00FF;
        case Addressing::ZeroPage_y:
            // The zero page address in the next byte is summed with the
            // contents of the y register.
            addr = bus.read(pc++);
            return (addr + y) & 0x00FF;

        case Addressing::Relative:
            // This one is only used in branching instructions. The next byte
            // contains a signed, 8-bit jump offset, which should be correctly
            // converted to a 16-bit value and summed with the address of the
            // instruction itself (which is PC minus 2 after reading the offset)
            // to obtain the absolute address to jump to.
            addr = bus.read(pc++);

            // To convert the jump offset to a 16-bit signed integer, we have
            // to check whether it is negative, which is indicated by the
            // value of the 7th bit. If it is, we have to set its high 8 bits
            // to 1s. This is enough for the address math to work out correctly.
            if(addr & 0x80) addr &= 0xFF00;
            addr += pc - 2;
            return addr;

        case Addressing::Absolute:
            // The following two bytes of the instruction are, in little endian
            // order, part of a 16-bit absolute address
            addr = bus.read(pc++);
            addr |= bus.read(pc++) << 8;
            return addr;
        case Addressing::Absolute_x:
            // The following two bytes of the instruction are, in little endian
            // order, part of a 16-bit absolute address, which has to be summed
            // with the contents of the x register. NOTE this may require an
            // aditional clock cycle if, after the addition with x, the address
            // crosses a page boundary
            addr = bus.read(pc++);
            addr |= bus.read(pc++) << 8;
            return addr + x;
        case Addressing::Absolute_y:
            // The following two bytes of the instruction are, in little endian
            // order, part of a 16-bit absolute address, which has to be summed
            // with the contents of the y register. NOTE this may require an
            // aditional clock cycle if, after the addition with y, the address
            // crosses a page boundary
            addr = bus.read(pc++);
            addr |= bus.read(pc++) << 8;
            return addr + y;

        case Addressing::Indirect:
            // The following two bytes of the instruction are, in little endian
            // order, part of a 16-bit pointer to the real absolute address.
            // BUG this addressing mode has a bug in the original hardware! It
            // has to do with cycles, which I'm not dealing with right now, but
            // once I do, I better take care of that
            ptr = bus.read(pc++);
            ptr |= bus.read(pc++) << 8;
            addr = bus.read(ptr);
            addr |= bus.read(ptr + 1) << 8;
            return addr;

        case Addressing::Indirect_x:
            // A zero page address is in the following byte. Summing it with
            // the contents of the x register (with zero page wrap around),
            // we get a zero page pointer to the real, 16-bit absolute address
            ptr = bus.read(pc++);
            ptr = (ptr + x) & 0x00FF;
            addr = bus.read(ptr);
            addr |= bus.read((ptr + 1) & 0x00FF);
            return addr;
        case Addressing::Indirect_y:
            // A zero page address is in the following byte. It points to the
            // real, 16-bit absolute address, which is summed with the contents
            // of the y register to give the final result. NOTE this may
            // require an extra clock cycle if, after addition with y, the
            // address crosses a page boundary
            ptr = bus.read(pc++);
            addr = bus.read(ptr);
            addr |= bus.read((ptr + 1) & 0x00FF);
            return addr + y;
    }
    return 0; // unreachable
}

uint8_t Processor::get_data() {
    uint16_t addr;
    switch(addr_mode) {
        case Addressing::Null:
            // Addresing mode wasn't properly initialized
            std::cerr << "Invalid addressing mode!\n";
            return 0;
        case Addressing::Implicit:
            // No need to fetch data
            return 0;
        case Addressing::Accumulator:
            // Accumulator is used as an immediate argument
            return acc;
        case Addressing::Immediate:
            // The data is the byte following the instruction
            return bus.read(pc++);
        case Addressing::Relative:
            // It makes no sense to fetch data here, given that the only
            // instructions that use this mode are branching instructions,
            // which only require addresses to work
            return 0;
        default:
            // For the other addressing modes, it's really just a matter of
            // fetching an 8-bit value from the address they specify
            addr = get_address();
            return bus.read(addr);
    }
    return 0; // unreachable
}
