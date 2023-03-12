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
            // Construct a processor connected to an emulator object
            Processor(Emulator &bus);

            // Reset the state of the CPU
            void reset_state();

            // Run a single cycle of execution
            void single_step();

            // Show the current state of all registers
            void show_registers() const;

            // Show next opcode to be executed
            void show_opcode() const;

            // Show values on the stack, from top to bottom
            void show_stack() const;

        private:
            // Reference to the current emulator object, which acts as the main
            // data bus. Its read and write methods are the primary way for the
            // CPU to communicate with other devices in the system
            Emulator &bus;

            // Index registers: most commonly used to hold counters or offsets
            uint8_t x = 0, y = 0;

            // Accumulator register: used by arithmetic and logic operations
            uint8_t acc = 0;

            // Program counter: stores the address of the next instruction to
            // be executed. Ordinarily, it increases linearly through RAM, but
            // it can be (and is) modified directly for control flow
            uint16_t pc = 0;

            // The base address of the stack in RAM: it is the last possible
            // address the stack may occupy. The real utility that it has is
            // that you can bitwise OR it with the stack pointer to get the
            // absolute address it corresponds to
            static const uint16_t stack_base = 0x0100;

            // Stack pointer: holds the low byte of the address of the next 
            // free position of the (descending!) stack in RAM
            uint8_t stack_ptr = 0xFF;

            // Push a byte on the stack
            void stack_push(uint8_t byte);

            // Pull a byte from the stack
            uint8_t stack_pull();

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

            // Get the value of a flag from the status register
            uint8_t get_flag(Flag flag) const;

            // Set the value of a flag in the status register
            void set_flag(Flag flag, bool state);

            // Addressing modes are basically the different "flavors" the same
            // instruction may come in. They specify how many additional bytes
            // will be needed beyond the opcode and in which way those bytes,
            // if present, will be used by the instruction. They can be
            // uniquely determined from the opcode. Here, they are represented
            // as an enumeration and a set of addressing-mode-aware functions
            // for fetching addresses and data.

            enum class Addressing : uint8_t {
                Null        , // initial, invalid addressing mode
                Implied     ,
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

            // There are a total of 56 instructions available in the processor.
            // Each one of them should work with multiple addressing modes,
            // hence why the modes are mostly abstracted behind the previous 
            // methods.

            // Load and store instructions:
            void inst_lda();
            void inst_ldx();
            void inst_ldy();
            void inst_sta();
            void inst_stx();
            void inst_sty();

            // Register transfer instructions:
            void inst_tax();
            void inst_tay();
            void inst_txa();
            void inst_tya();

            // Stack instructions:
            void inst_tsx();
            void inst_txs();
            void inst_pha();
            void inst_php();
            void inst_pla();
            void inst_plp();

            // Logic instructions:
            void inst_and();
            void inst_eor();
            void inst_ora();
            void inst_bit();

            // Increment instructions:
            void inst_inc();
            void inst_inx();
            void inst_iny();

            // Decrement instructions:
            void inst_dec();
            void inst_dex();
            void inst_dey();

            // Shift instructions:
            void inst_asl();
            void inst_lsr();
            void inst_rol();
            void inst_ror();

            // Jump instructions:
            void inst_jmp();
            void inst_jsr();
            void inst_rts();

            // Branch instructions:
            void inst_bcc();
            void inst_bcs();
            void inst_beq();
            void inst_bmi();
            void inst_bne();
            void inst_bpl();
            void inst_bvc();
            void inst_bvs();

            // Flag set instructions:
            void inst_sec() { set_flag(Flag::Carry            , true); }
            void inst_sei() { set_flag(Flag::InterruptDisable , true); }
            void inst_sed() { set_flag(Flag::Decimal          , true); }

            // Flag clear intructions:
            void inst_clc() { set_flag(Flag::Carry            , false); }
            void inst_cli() { set_flag(Flag::InterruptDisable , false); }
            void inst_cld() { set_flag(Flag::Decimal          , false); }
            void inst_clv() { set_flag(Flag::Overflow         , false); }

            // TODO:
            // - arithmetic instructions
            // - interrupt related instructions (BRK and RTI)
    };
}

#endif // NES_PROCESSOR_HPP
