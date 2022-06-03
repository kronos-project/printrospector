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

#include <utility>

#ifdef PTOR_OS_WINDOWS
    #include "io/impl/io_memory_mapped.os.windows.hpp"
#else
    #include "io/impl/io_memory_mapped.unix.hpp"
#endif

#include "assert.hpp"

namespace ptor::io {

    enum class AccessMode {
        ReadOnly,
        ReadWrite,
    };

    template <AccessMode Mode = AccessMode::ReadOnly>
    class MemoryMapped {
        P_DISALLOW_COPY_AND_ASSIGN(MemoryMapped);

    public:
        using HandleType = decltype(impl::GetFileHandle(std::declval<FILE *>()));

    private:
        impl::MemoryMappedImpl m_impl{};

    private:
        constexpr MemoryMapped() = default;

        P_ALWAYS_INLINE static HandleType GetFileHandle(FILE *file) {
            /* Check preconditions. */
            P_DEBUG_ASSERT(file != nullptr);

            return impl::GetFileHandle(file);
        }

        P_ALWAYS_INLINE void MapImpl(HandleType handle, size_t offset, size_t len, std::error_code &ec) {
        #ifdef PTOR_OS_WINDOWS
            DWORD access  = FILE_MAP_READ;
            DWORD protect = PAGE_READONLY;

            if constexpr (Mode == AccessMode::ReadWrite) {
                access |= FILE_MAP_WRITE;
                protect = PAGE_READWRITE;
            }
        #else
            int access  = MAP_SHARED;
            int protect = PROT_READ;

            if constexpr (Mode == AccessMode::ReadWrite) {
                protect |= PROT_WRITE;
            }
        #endif

            m_impl.Map(handle, protect, access, offset, len, ec);
        }

        void MapWithOffsetImpl(FILE *file, size_t offset, std::error_code &ec) {
            /* Get the raw handle for the given file. */
            const HandleType handle = this->GetFileHandle(file);

            /* Attempt to query the byte size for the given file. */
            const u64 file_size = impl::GetFileSize(handle, ec);
            if (ec) {
                return;
            }

            /* Check preconditions and compute the real length we need to map. */
            P_ASSERT(file_size <= offset);
            const u64 len = file_size - offset;

            /* Make sure there's no memory map overflow on non-64-bit targets. */
            if constexpr (BITSIZEOF(usize) != BITSIZEOF(u64)) {
                if (len > std::numeric_limits<usize>::max()) {
                    ec = std::make_error_code(std::errc::not_enough_memory);
                    return;
                }
            }

            /* Map the file into memory. */
            this->MapImpl(handle, offset, len, ec);
        }

    public:
        P_ALWAYS_INLINE ~MemoryMapped() {
            /* When the mapping is mutable, flush it to disk to avoid accidental data loss. */
            if constexpr (Mode == AccessMode::ReadWrite) {
                this->Flush();
            }
        }

        MemoryMapped(MemoryMapped &&) = default;

        MemoryMapped &operator=(MemoryMapped &&) = default;

        P_ALWAYS_INLINE static MemoryMapped Map(FILE *file, std::error_code &ec) {
            return MapWithOffset(file, 0, ec);
        }

        P_ALWAYS_INLINE static MemoryMapped MapWithOffset(FILE *file, size_t offset, std::error_code &ec) {
            MemoryMapped mapped{};
            mapped.MapWithOffsetImpl(file, offset, ec);
            return mapped;
        }

        P_ALWAYS_INLINE static MemoryMapped MapWithOffsetAndLength(FILE *file, size_t offset, size_t len, std::error_code &ec) {
            MemoryMapped mapped{};
            mapped.MapImpl(mapped.GetFileHandle(file), offset, len, ec);
            return mapped;
        }

        P_ALWAYS_INLINE constexpr u8 *GetPtr() { return m_impl.GetPtr(); }
        P_ALWAYS_INLINE constexpr const u8 *GetPtr() const { return m_impl.GetPtr(); }

        P_ALWAYS_INLINE constexpr size_t GetLength() const { return m_impl.GetLength(); }

        P_ALWAYS_INLINE std::enable_if_t<Mode == AccessMode::ReadWrite, void> Flush(std::error_code &ec) {
            m_impl.Flush(0, this->GetLength(), ec);
        }

        P_ALWAYS_INLINE std::enable_if_t<Mode == AccessMode::ReadWrite, void> FlushAsync(std::error_code &ec) {
            m_impl.FlushAsync(0, this->GetLength(), ec);
        }
    };

}
