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

#include <bitset>
#include <cstdint>
#include <iostream>

#include "emulator.hpp"
#include "processor.hpp"

using namespace nes;

Processor::Processor(Emulator &bus) : bus(bus) {
    // The initial value for the PC has to be fetched from 0xFFFC
    pc = bus.read(0xFFFC);
    pc |= bus.read(0xFFFD) << 8;
}

void Processor::reset_state() {
    // Reset all of the registers
    x = 0;
    y = 0;
    acc = 0;
    status = 0;
    stack_ptr = 0xFF;
    // The initial value for the PC has to be fetched from 0xFFFC
    pc = bus.read(0xFFFC);
    pc |= bus.read(0xFFFD) << 8;
    // Reset addressing mode to an initial, invalid state
    addr_mode = Addressing::Null;
}

void Processor::single_step() {
    addr_mode = Addressing::Null;
    uint8_t opcode = bus.read(pc++);
    switch(opcode) {
        // Testing stuff
        case 0x06:
            addr_mode = Addressing::ZeroPage;
            inst_asl();
            break;
        case 0x08:
            addr_mode = Addressing::Implied;
            inst_php();
            break;
        case 0x0A:
            addr_mode = Addressing::Accumulator;
            inst_asl();
            break;
        case 0x0E:
            addr_mode = Addressing::Absolute;
            inst_asl();
            break;
        case 0x11:
            addr_mode = Addressing::Indirect_y;
            inst_ora();
            break;
        case 0x29:
            addr_mode = Addressing::Immediate;
            inst_and();
            break;
        case 0x38:
            addr_mode = Addressing::Implied;
            inst_sec();
            break;
        case 0x3E:
            addr_mode = Addressing::Absolute_x;
            inst_rol();
            break;
        case 0x48:
            addr_mode = Addressing::Implied;
            inst_pha();
            break;
        case 0x56:
            addr_mode = Addressing::ZeroPage_x;
            inst_lsr();
            break;
        case 0x6C:
            addr_mode = Addressing::Indirect;
            inst_jmp();
            break;
        case 0x81:
            addr_mode = Addressing::Indirect_x;
            inst_sta();
            break;
        case 0x99:
            addr_mode = Addressing::Absolute_y;
            inst_sta();
            break;
        case 0xA0:
            addr_mode = Addressing::Immediate;
            inst_ldy();
            break;
        case 0xA2:
            addr_mode = Addressing::Immediate;
            inst_ldx();
            break;
        case 0xA5:
            addr_mode = Addressing::ZeroPage;
            inst_lda();
            break;
        case 0xA9:
            addr_mode = Addressing::Immediate;
            inst_lda();
            break;
        case 0xB6:
            addr_mode = Addressing::ZeroPage_y;
            inst_ldx();
            break;
        case 0xF0:
            addr_mode = Addressing::Relative;
            inst_beq();
            break;
    }
}

void Processor::show_registers() const {
    // I use printf here because printing hexadecimal numbers the C++ way
    // causes me physical pain
    printf("PC: 0x%04X\n", pc);
    printf("X: 0x%02X\n", x);
    printf("Y: 0x%02X\n", y);
    printf("A: 0x%02X\n", acc);
    printf("S: 0x%02X\n", stack_ptr);
    std::bitset<8> status_bits(status);
    std::cout << "P: 0b" << status_bits << '\n';
}

void Processor::show_opcode() const {
    uint8_t op = bus.read(pc);
    printf("Next opcode to be executed: 0x%02X\n", op);
}

void Processor::show_stack() const {
    bool first = true;
    uint16_t ptr = stack_base | stack_ptr;
    std::cout << "[";
    while(ptr < 0x01FF) {
        if(first) {
            printf("0x%02X", bus.read(++ptr));
            first = false;
        } else {
            printf(" 0x%02X", bus.read(++ptr));
        }
    }
    std::cout << "]\n";
}

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
    if(state) status |= f;
    else status &= ~f;
}

uint16_t Processor::get_address() {
    // Get an absolute address based on the addressing mode
    uint16_t address, ptr;
    switch(addr_mode) {
        case Addressing::Null:
            // Addressing mode wasn't properly initialized
            std::cerr << "Invalid addressing mode!\n";
            address = 0;
            break;
        case Addressing::Implied:
            // No absolute address to fetch
            address = 0;
            break;
        case Addressing::Accumulator:
            // No absolute address to fetch
            address = 0;
            break;
        case Addressing::Immediate:
            // No absolute address to fetch (here, we could also return the
            // address of the following byte, but this is fine for now)
            address = 0;
            break;

        case Addressing::ZeroPage:
            // A zero page address is stored in the next byte
            address = bus.read(pc++) & 0x00FF;
            break;
        case Addressing::ZeroPage_x:
            // The zero page address in the next byte is summed with the
            // contents of the x register.
            address = (bus.read(pc++) + x) & 0x00FF;
            break;
        case Addressing::ZeroPage_y:
            // The zero page address in the next byte is summed with the
            // contents of the y register.
            address = (bus.read(pc++) + y) & 0x00FF;
            break;

        case Addressing::Relative:
            // This one is only used in branching instructions. The next byte
            // contains a signed, 8-bit jump offset, which should be correctly
            // converted to a 16-bit value and summed with the address of the
            // instruction itself (which is PC minus 2 after reading the offset)
            // to obtain the absolute address to jump to.
            address = bus.read(pc++);

            // To convert the jump offset to a 16-bit signed integer, we have
            // to check whether it is negative, which is indicated by the
            // value of the 7th bit. If it is, we have to set its high 8 bits
            // to 1s. This is enough for the address math to work out correctly.
            if(address & 0x80) address |= 0xFF00;
            address += pc - 2;
            break;

        case Addressing::Absolute:
            // The following two bytes of the instruction are, in little endian
            // order, part of a 16-bit absolute address
            address = bus.read(pc++);
            address |= bus.read(pc++) << 8;
            break;
        case Addressing::Absolute_x:
            // The following two bytes of the instruction are, in little endian
            // order, part of a 16-bit absolute address, which has to be summed
            // with the contents of the x register. NOTE this may require an
            // aditional clock cycle if, after the addition with x, the address
            // crosses a page boundary
            address = bus.read(pc++);
            address |= bus.read(pc++) << 8;
            address += x;
            break;
        case Addressing::Absolute_y:
            // The following two bytes of the instruction are, in little endian
            // order, part of a 16-bit absolute address, which has to be summed
            // with the contents of the y register. NOTE this may require an
            // aditional clock cycle if, after the addition with y, the address
            // crosses a page boundary
            address = bus.read(pc++);
            address |= bus.read(pc++) << 8;
            address += y;
            break;

        case Addressing::Indirect:
            // The following two bytes of the instruction are, in little endian
            // order, part of a 16-bit pointer to the real absolute address.
            ptr = bus.read(pc++);
            ptr |= bus.read(pc++) << 8;

            // This addressing mode had a bug in the original hardware! When
            // adding 1 to the pointer would cross a page boundary, the high
            // byte of the target address is incorrectly fetched from the
            // beginning of the pointer's current page. For compatibility with
            // the NES, this is a bug we have to reproduce
            address = bus.read(ptr);
            if((ptr & 0x00FF) == 0x00FF)
                address |= bus.read(ptr & 0xFF00) << 8;
            else
                address |= bus.read(ptr + 1) << 8;
            break;

        case Addressing::Indirect_x:
            // A zero page address is in the following byte. Summing it with
            // the contents of the x register (with zero page wrap around),
            // we get a zero page pointer to the real, 16-bit absolute address
            ptr = bus.read(pc++);
            ptr = (ptr + x) & 0x00FF;
            address = bus.read(ptr);
            address |= bus.read((ptr + 1) & 0x00FF) << 8;
            break;
        case Addressing::Indirect_y:
            // A zero page address is in the following byte. It points to the
            // real, 16-bit absolute address, which is summed with the contents
            // of the y register to give the final result. NOTE this may
            // require an extra clock cycle if, after addition with y, the
            // address crosses a page boundary
            ptr = bus.read(pc++);
            address = bus.read(ptr);
            address |= bus.read((ptr + 1) & 0x00FF) << 8;
            address += y;
            break;
    }
    return address;
}

uint8_t Processor::get_data(uint16_t *address) {
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
            if(address != nullptr) 
                *address = addr;
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

// Increment instructions:

void Processor::inst_inc() {
    // Increment the memory location at the given address
    uint16_t addr;
    uint8_t data = get_data(&addr);
    ++data;
    bus.write(addr, data);
    set_flag(Flag::Zero, data == 0);
    set_flag(Flag::Negative, data & 0x80);
}

void Processor::inst_inx() {
    // Increment the x register
    ++x;
    set_flag(Flag::Zero, x == 0);
    set_flag(Flag::Negative, x & 0x80);
}

void Processor::inst_iny() {
    // Increment the y register
    ++y;
    set_flag(Flag::Zero, y == 0);
    set_flag(Flag::Negative, y & 0x80);
}

// Decrement instructions:

void Processor::inst_dec() {
    // Decrement the memory location at the given address
    uint16_t addr;
    uint8_t data = get_data(&addr);
    --data;
    bus.write(addr, data);
    set_flag(Flag::Zero, data == 0);
    set_flag(Flag::Negative, data & 0x80);
}

void Processor::inst_dex() {
    // Decrement the x register
    --x;
    set_flag(Flag::Zero, x == 0);
    set_flag(Flag::Negative, x & 0x80);
}

void Processor::inst_dey() {
    // Decrement the y register
    --y;
    set_flag(Flag::Zero, y == 0);
    set_flag(Flag::Negative, y & 0x80);
}

// Shift instructions:

void Processor::inst_asl() {
    // Arithmetic shift to the left of the memory location at the given address
    // or the accumulator, depending on the addressing mode
    uint16_t addr;
    uint8_t data = get_data(&addr);
    set_flag(Flag::Carry, data & 0x80);
    data <<= 1;
    set_flag(Flag::Negative, data & 0x80);
    if(addr_mode != Addressing::Accumulator)
        bus.write(addr, data);
    else
        acc = data;
}

void Processor::inst_lsr() {
    // Logical shift to the right of the memory location at the given address
    // or the accumulator, depending on the addressing mode
    uint16_t addr;
    uint8_t data = get_data(&addr);
    set_flag(Flag::Carry, data & 0x01);
    data >>= 1;
    set_flag(Flag::Negative, data & 0x80);
    if(addr_mode != Addressing::Accumulator)
        bus.write(addr, data);
    else
        acc = data;
}

void Processor::inst_rol() {
    // Rotate to the left the memory location at the given address or the
    // accumulator, depending on the addressing mode
    uint16_t addr;
    uint8_t data = get_data(&addr);
    uint8_t bit7 = data & 0x80;
    data <<= 1;
    // The bit that was shifted out (0) is filled with the current value of
    // the carry flag
    data |= get_flag(Flag::Carry);
    set_flag(Flag::Carry, bit7);
    set_flag(Flag::Negative, data & 0x80);
    if(addr_mode != Addressing::Accumulator)
        bus.write(addr, data);
    else
        acc = data;
}

void Processor::inst_ror() {
    // Rotate to the right the memory location at the given address or the
    // accumulator, depending on the addressing mode
    uint16_t addr;
    uint8_t data = get_data(&addr);
    uint8_t bit0 = data & 0x01;
    data >>= 1;
    // The bit that was shifted out (7) is filled with the current value of
    // the carry flag
    data |= get_flag(Flag::Carry) << 7;
    set_flag(Flag::Carry, bit0);
    set_flag(Flag::Negative, data & 0x80);
    if(addr_mode != Addressing::Accumulator)
        bus.write(addr, data);
    else
        acc = data;
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
    stack_push(pc & 0x00FF);
    stack_push((pc & 0xFF00) >> 8);
    pc = subroutine;
}

void Processor::inst_rts() {
    // Return from subroutine: pops the stack for the address to return to
    pc = stack_pull() << 8;
    pc |= stack_pull();
}

// Branch instructions:

void Processor::inst_bcc() {
    // Branch if carry flag is clear
    if(!get_flag(Flag::Carry)) {
        pc = get_address();
    }
}

void Processor::inst_bcs() {
    // Branch if carry flag is set
    if(get_flag(Flag::Carry)) {
        pc = get_address();
    }
}

void Processor::inst_beq() {
    // Branch if zero flag is set
    if(get_flag(Flag::Zero)) {
        pc = get_address();
    }
}

void Processor::inst_bmi() {
    // Branch if negative flag is set
    if(get_flag(Flag::Negative)) {
        pc = get_address();
    }
}

void Processor::inst_bne() {
    // Branch if zero flag is clear
    if(!get_flag(Flag::Zero)) {
        pc = get_address();
    }
}

void Processor::inst_bpl() {
    // Branch if negative flag is clear
    if(!get_flag(Flag::Negative)) {
        pc = get_address();
    }
}

void Processor::inst_bvc() {
    // Branch if overflow flag is clear
    if(!get_flag(Flag::Overflow)) {
        pc = get_address();
    }
}

void Processor::inst_bvs() {
    // Branch if overflow flag is set
    if(get_flag(Flag::Overflow)) {
        pc = get_address();
    }
}
