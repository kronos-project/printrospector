/*
 * Copyright (c) 2022 Valentin B.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <span>

#include "io/io_binary_buffer.hpp"
#include "wad/wad_types.hpp"

namespace ptor::wad {

    Header ReadHeader(io::BinaryBuffer &buffer);

    File ReadFile(io::BinaryBuffer &buffer);

    P_ALWAYS_INLINE std::span<u8> GetFileContents(const File &file, u8 *archive) {
        size_t file_size = file.compressed ? file.compressed_size : file.uncompressed_size;
        return {archive + file.start_offset, file_size};
    }

}
