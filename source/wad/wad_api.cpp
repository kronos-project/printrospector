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

#include "wad/wad_api.hpp"

#include "assert.hpp"

namespace ptor::wad {

    namespace {

        P_ALWAYS_INLINE std::string_view ReadPath(io::BinaryBuffer &buffer) {
            size_t len = buffer.ReadValue<u32>();
            char *data = reinterpret_cast<char *>(buffer.GetCursorPtr());
            buffer.RewindCursor(buffer.GetCursorOffset() + static_cast<ptrdiff_t>(len));

            P_ASSERT(data[len - 1] == 0, "corrupt file path string");
            return {data, len};
        }

    }

    Header ReadHeader(io::BinaryBuffer &buffer) {
        /* Validate the KIWAD archive magic and discard it. */
        P_ASSERT(std::memcmp(buffer.GetCursorPtr(), ArchiveMagic, 5) == 0, "archive does not start with KIWAD magic");
        buffer.RewindCursor(5);

        /* Read the header fields. */
        const u32 version    = buffer.ReadValue<u32>();
        const u32 file_count = buffer.ReadValue<u32>();
        const auto flags     = (version >= 2) ? static_cast<ArchiveFlags>(buffer.ReadValue<u8>()) : ArchiveFlag_None;

        return {version, file_count, flags};
    }

    File ReadFile(io::BinaryBuffer &buffer) {
        /* Read the file metadata fields. */
        const u32 start_offset      = buffer.ReadValue<u32>();
        const u32 uncompressed_size = buffer.ReadValue<u32>();
        const u32 compressed_size   = buffer.ReadValue<u32>();
        const bool compressed       = buffer.ReadValue<u8>() != 0;
        const u32 checksum          = buffer.ReadValue<u32>();
        const fs::path path         = ReadPath(buffer);

        return {start_offset, uncompressed_size, compressed_size, compressed, checksum, path};
    }

}
