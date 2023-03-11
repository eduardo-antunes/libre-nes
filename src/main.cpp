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

#include <vector>
#include <cstdint>
#include <iostream>
#include "emulator.hpp"

int main() {
    nes::Emulator nes_emu;
    std::vector<uint8_t> prog { 0xA9, 0x00, 0xF0, 0xFE };

    nes_emu.load_prog(prog, 2);
    nes_emu.start();
    return 0;
}
