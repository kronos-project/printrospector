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

#include "ptor_types.hpp"

namespace ptor::wad {

    constexpr inline const char *ArchiveMagic = "KIWAD";

    /* Implementation-defined bits for handling WAD archives. */
    enum ArchiveFlags : u8 {
        ArchiveFlag_None           = 0 << 0,

        ArchiveFlag_MemoryMapped   = 1 << 0,
        ArchiveFlag_PrefetchHandle = 1 << 1,
    };

    /* Header of every archive file. */
    struct Header {
        u32 version;                /* The version of the WAD format in use.   */
        u32 file_count;             /* The amount of archived files contained. */
        ArchiveFlags archive_flags; /* Optional; Only present in version >= 2. */
    };

    /* Metadata for an archived file. */
    struct File {
        u32 start_offset;      /* The starting offset of the file contents.     */
        u32 uncompressed_size; /* The uncompressed size of the file contents.   */
        u32 compressed_size;   /* The compressed size of the file, if relevant. */
        bool compressed;       /* Whether the file contents are compressed.     */
        u32 checksum;          /* The CRC32 checksum of the file contents.      */
        fs::path path;         /* The archive-relative path of the file.        */
    };

}
