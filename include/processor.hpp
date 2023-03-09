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

#ifndef NES_PROCESSOR_HPP
#define NES_PROCESSOR_HPP

#include <cstdint>

namespace nes { class Emulator; } // stupid forward declaration :)

// This class represents the processor used by the NES, a minor variation of
// the classic 6502 processor. In comparison to the chip-8 "processor" (my
// previous emulation project), it is one heck of a lot more complicated.

namespace nes {
    class Processor {
        public:
            Processor(Emulator &bus) : bus(bus) {}

        private:
            // Starting address of the stack in RAM
            uint16_t stack_start = 0x01FF;

            // Reference to the external world, for communication
            Emulator &bus;

            // Index registers: most commonly used to hold counters or offsets
            uint8_t x = 0, y = 0;

            // Accumulator register: used by arithmetic and logic operations
            uint8_t acc = 0;

            // Program counter: stores the address of the next instruction to
            // be executed. Ordinarily, it increases linearly through RAM, but
            // it can be (and is) modified directly for control flow
            uint16_t pc = 0x0200;

            // Stack pointer: holds the low byte of the address of the next 
            // free position of the (descending!) stack in RAM
            uint8_t stack_ptr = 0xFF;

            // Status register: used to record information about the
            // results of previously executed instructions
            uint8_t status = 0;

            // Flags indicated by the status register
            enum class Flag : uint8_t {
                Carry            = (1 << 0),
                Zero             = (1 << 1),
                InterruptDisable = (1 << 2),
                Decimal          = (1 << 3), // no effect in the NES
                Break            = (1 << 4), // no CPU effect
                Unused           = (1 << 5), // no CPU effect
                Overflow         = (1 << 6),
                Negative         = (1 << 7),
            };

            // Get value of a flag from the status register
            uint8_t get_flag(Flag flag) const;

            // Set value of a flag in the status register
            void set_flag(Flag flag, bool state = true);

            // Addressing modes are basically the different "flavors" the same
            // instruction may come in. They specify how many additional bytes
            // will be needed beyond the opcode and in which way those bytes,
            // if present, will be used by the instruction. They can be
            // uniquely determined from the opcode. Here, they are represented
            // as an enumeration and a set of addressing-mode-aware functions
            // for fetching addresses and data.

            enum class Addressing : uint8_t {
                Null        , // initial, invalid addressing mode
                Implicit    ,
                Accumulator ,
                Immediate   ,
                ZeroPage    ,
                ZeroPage_x  ,
                ZeroPage_y  ,
                Relative    ,
                Absolute    ,
                Absolute_x  ,
                Absolute_y  ,
                Indirect    ,
                Indirect_x  ,
                Indirect_y  ,
            };

            // Current addressing mode, should be reset for every instruction
            Addressing addr_mode = Addressing::Null;

            // Based on the current addressing mode, get an absolute address
            // for the current instruction to work with
            uint16_t get_address();

            // Based on the current addressing mode, get an 8-bit value for the
            // current instruction to work with
            uint8_t get_data();
    };
}

#endif // NES_PROCESSOR_HPP
