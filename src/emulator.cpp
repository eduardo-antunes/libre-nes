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
#include "emulator.hpp"

using namespace nes;

uint8_t Emulator::read(uint16_t addr) const {
    if(addr >= 0x0000 && addr <= 0x1FFF)
        // RAM is mirrored throught this range
        return ram[addr & 0x07FF];
    return 0;
}

void Emulator::write(uint16_t addr, uint8_t data) {
    if(addr >= 0x0000 && addr <= 0x1FFF)
        // RAM is mirrored throught this range
        ram[addr & 0x07FF] = data;
}
