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

#include <cstdio>
#include <system_error>

#include <io.h>
#include <windows.h>

#include "ptor_defines.hpp"
#include "ptor_types.hpp"

namespace ptor::io::impl {

    class MemoryMappedImpl final {
        P_DISALLOW_COPY_AND_ASSIGN(MemoryMappedImpl);

    private:
        /* Span information for the mapped memory region. */
        u8 *m_ptr = nullptr;
        size_t m_len = 0;

        /* The underlying handle for the mapped file. */
        HANDLE m_handle = INVALID_HANDLE_VALUE;

    public:
        constexpr MemoryMappedImpl() = default;

        ~MemoryMappedImpl();

        MemoryMappedImpl(MemoryMappedImpl &&);

        MemoryMappedImpl &operator=(MemoryMappedImpl &&);

        P_ALWAYS_INLINE constexpr u8 *GetPtr() { return m_ptr; }
        P_ALWAYS_INLINE constexpr const u8 *GetPtr() const { return m_ptr; }

        P_ALWAYS_INLINE constexpr bool IsMapped() const { return m_ptr != nullptr; }

        P_ALWAYS_INLINE constexpr size_t GetLength() const { return m_len; }

        void Map(HANDLE handle, DWORD protect, DWORD access, size_t offset, size_t len, std::error_code &ec);

        void Unmap();

        void Flush(size_t offset, size_t len, std::error_code &ec);

        void FlushAsync(size_t offset, size_t len, std::error_code &ec);
    };

    u64 GetFileSize(HANDLE handle, std::error_code &ec);

    P_ALWAYS_INLINE HANDLE GetFileHandle(FILE *file) {
        return reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file)));
    }

}
