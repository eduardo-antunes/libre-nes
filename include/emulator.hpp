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

#ifndef NES_EMULATOR_HPP
#define NES_EMULATOR_HPP

#include <array>
#include "processor.hpp"

// This class represents both the console itself, holding a list of its major
// components, and the main data bus, which is used by these components to
// communicate with each other.

namespace nes {
    class Emulator {
        public:
            // Upon construction, we have to somehow "connect" the components
            // that need to communicate with each other to the main bus. This
            // is done by passing a reference to the current emulator object to
            // the constructors of these components, which is then used to read
            // from and write to the main data bus via the following methods.
            Emulator() : cpu(*this) {}

            // Read from the main data bus
            uint8_t read(uint16_t addr) const;

            // Write to the main data bus
            void write(uint16_t addr, uint8_t data);

        private:
            // The 6502-like processor used by the NES
            Processor cpu;

            // 2KiB of RAM (riches beyond wonders!). Its address space spans,
            // however, a total of 8KiB, which is achieved through mirroring.
            // That is implemented in the read and write methods.
            std::array<uint8_t, 2048> ram;
    };
}

#endif // NES_EMULATOR_HPP
