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

void Processor::stack_push(uint8_t byte) {
    // NOTE remember, the stack is descending!
    // We have to decrement the stack pointer here
    uint16_t addr = stack_base | stack_ptr;
    bus.write(addr, byte);
    --stack_ptr;
}

uint8_t Processor::stack_pull() {
    // NOTE remember, the stack is descending!
    // We have to increment the stack pointer here
    ++stack_ptr;
    uint16_t addr = stack_base | stack_ptr;
    return bus.read(addr);
}

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
        case Addressing::Implied:
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
        case Addressing::Implied:
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

// Load and store instructions:

void Processor::inst_lda() {
    // Load given data into the accumulator
    acc = get_data();
    set_flag(Flag::Zero, acc == 0);
    set_flag(Flag::Negative, acc & 0x80);
}

void Processor::inst_ldx() {
    // Load given data into the x register
    x = get_data();
    set_flag(Flag::Zero, x == 0);
    set_flag(Flag::Negative, x & 0x80);
}

void Processor::inst_ldy() {
    // Load given data into the y register
    y = get_data();
    set_flag(Flag::Zero, y == 0);
    set_flag(Flag::Negative, y & 0x80);
}

void Processor::inst_sta() {
    // Store the contents of the accumulator into the given address
    uint16_t addr = get_address();
    bus.write(addr, acc);
}

void Processor::inst_stx() {
    // Store the contens of the x register into the given address
    uint16_t addr = get_address();
    bus.write(addr, x);
}

void Processor::inst_sty() {
    // Store the contens of the y register into the given address
    uint16_t addr = get_address();
    bus.write(addr, y);
}

// Register transfer instructions:

void Processor::inst_tax() {
    // Copy the accumulator into the x register
    x = acc;
    set_flag(Flag::Zero, x == 0);
    set_flag(Flag::Negative, x & 0x80);
}

void Processor::inst_tay() {
    // Copy the accumulator into the y register
    y = acc;
    set_flag(Flag::Zero, y == 0);
    set_flag(Flag::Negative, y & 0x80);
}

void Processor::inst_txa() {
    // Copy the x register into the accumulator
    acc = x;
    set_flag(Flag::Zero, acc == 0);
    set_flag(Flag::Negative, acc & 0x80);
}

void Processor::inst_tya() {
    // Copy the y register into the accumulator
    acc = y;
    set_flag(Flag::Zero, acc == 0);
    set_flag(Flag::Negative, acc & 0x80);
}

// Stack instructions:

void Processor::inst_tsx() {
    // Transfer stack pointer to the x register
    x = stack_ptr;
    set_flag(Flag::Zero, x == 0);
    set_flag(Flag::Negative, x & 0x80);
}

void Processor::inst_txs() {
    // Transfer the contents of the x register to the stack pointer
    stack_ptr = x;
}

void Processor::inst_pha() {
    // Push the value of the accumulator on the stack
    stack_push(acc);
}

void Processor::inst_php() {
    // Push the contents of the status register on the stack
    stack_push(status);
}

void Processor::inst_pla() {
    // Pull a byte from the stack and put it into the accumulator
    acc = stack_pull();
    set_flag(Flag::Zero, acc == 0);
    set_flag(Flag::Negative, acc & 0x80);
}

void Processor::inst_plp() {
    // Pull a byte from the stack and put it into the status register
    status = stack_pull();
}

// Logic instructions:

void Processor::inst_and() {
    // Bitwise AND with the accumulator
    uint8_t data = get_data();
    acc &= data;
    set_flag(Flag::Zero, acc == 0);
    set_flag(Flag::Negative, acc & 0x80);
}

void Processor::inst_eor() {
    // Bitwise XOR with the accumulator
    uint8_t data = get_data();
    acc ^= data;
    set_flag(Flag::Zero, acc == 0);
    set_flag(Flag::Negative, acc & 0x80);
}

void Processor::inst_ora() {
    // Bitwise OR with the accumulator
    uint8_t data = get_data();
    acc |= data;
    set_flag(Flag::Zero, acc == 0);
    set_flag(Flag::Negative, acc & 0x80);
}

void Processor::inst_bit() {
    // Bitwise AND with the accumulator, but the result is not kept. It is
    // instead used to set the zero, negative and overflow flags
    uint8_t data = get_data();
    data = acc & data;
    set_flag(Flag::Zero, data == 0);
    set_flag(Flag::Overflow, data & 0x40);
    set_flag(Flag::Negative, data & 0x80);
}

// Jump instructions:

void Processor::inst_jmp() {
    // Unconditional jump to the given address
    pc = get_address();
}

void Processor::inst_jsr() {
    // Jump to subroutine: it's pretty similar to the unconditional jump, except
    // it pushes the address of the following instruction on the stack, so that
    // the program can return to it after the subroutine is done
    uint16_t subroutine = get_address();
    stack_push(pc);
    pc = subroutine;
}

void Processor::inst_rts() {
    // Return from subroutine: pops the stack for the address to return to
    pc = stack_pull();
}
