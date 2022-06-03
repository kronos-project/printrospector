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

#include "io/impl/io_memory_mapped.os.windows.hpp"

#include "assert.hpp"
#include "util/util_alignment.hpp"
#include "util/util_scope_guard.hpp"

namespace ptor::io::impl {

    namespace {

        constexpr inline size_t DwordBits = BITSIZEOF(DWORD);

        P_ALWAYS_INLINE constexpr DWORD GetHigh(size_t value) {
            return value >> DwordBits;
        }

        P_ALWAYS_INLINE constexpr DWORD GetLow(size_t value) {
            return value & P_LSBLL(DwordBits);
        }

        P_ALWAYS_INLINE constexpr u64 JoinHalves(DWORD high, DWORD low) {
            return (static_cast<u64>(high) << DwordBits) | static_cast<u64>(low);
        }

        P_ALWAYS_INLINE size_t GetAllocationGranularity() {
            SYSTEM_INFO info;
            ::GetSystemInfo(std::addressof(info));
            return info.dwAllocationGranularity;
        }

        P_ALWAYS_INLINE std::error_code GetLastOsError() {
            return {static_cast<int>(::GetLastError()), std::system_category()};
        }

        bool ProtectionSupported(HANDLE file, DWORD protection) {
            HANDLE handle = ::CreateFileMappingW(file, nullptr, protection, 0, 0, nullptr);
            P_ON_SCOPE_EXIT { if (handle != nullptr) { ::CloseHandle(handle); } };

            return handle != nullptr;
        }

    }

    MemoryMappedImpl::~MemoryMappedImpl() {
        this->Unmap();
    }

    MemoryMappedImpl::MemoryMappedImpl(MemoryMappedImpl &&rhs)
        : m_ptr{rhs.m_ptr}, m_len{rhs.m_len}, m_handle{rhs.m_handle}
    {
        /* Reset rhs back into the default object state. */
        rhs.m_ptr    = nullptr;
        rhs.m_len    = 0;
        rhs.m_handle = INVALID_HANDLE_VALUE;
    }

    MemoryMappedImpl &MemoryMappedImpl::operator=(MemoryMappedImpl &&rhs) {
        /* First, remove the existing mapping from the target. */
        this->Unmap();

        /* Move the state in rhs over. */
        m_ptr    = rhs.m_ptr;
        m_len    = rhs.m_len;
        m_handle = rhs.m_handle;

        /* Reset rhs back into the default object state. */
        rhs.m_ptr    = nullptr;
        rhs.m_len    = 0;
        rhs.m_handle = INVALID_HANDLE_VALUE;
    }

    void MemoryMappedImpl::Map(HANDLE handle, DWORD protect, DWORD access, size_t offset, size_t len, std::error_code &ec) {
        /* Reset the error code back into a successful state. */
        ec.clear();

        /* Compute offset and length with respect to allocation granularity. */
        const size_t aligned_offset = util::AlignDown(offset, GetAllocationGranularity());
        const size_t alignment      = offset - aligned_offset;
        const size_t aligned_len    = len + alignment;

        /* Explicitly check for len 0 as we're not allowed to allocate that. */
        if (aligned_len == 0) {
            return;
        }

        /* Create the file mapping and ensure the operation succeeded. */
        HANDLE mapping = ::CreateFileMappingW(handle, nullptr, protect, 0, 0, nullptr);
        P_ON_SCOPE_EXIT { ::CloseHandle(mapping); };
        if (mapping == nullptr) {
            ec = GetLastOsError();
            return;
        }

        /* Memory-map the file view and ensure the operation succeeded. */
        auto *ptr = ::MapViewOfFile(mapping, access, GetHigh(aligned_offset), GetLow(aligned_offset), aligned_len);
        if (ptr == nullptr) {
            ec = GetLastOsError();
            return;
        }

        /* Duplicate the supplied handle so that the buffer alone can manage one. */
        HANDLE new_handle = nullptr;
        HANDLE cur_proc   = ::GetCurrentProcess();
        if (::DuplicateHandle(cur_proc, handle, cur_proc, std::addressof(new_handle), 0, 0, DUPLICATE_SAME_ACCESS) == 0) {
            ::UnmapViewOfFile(ptr);
            ec = GetLastOsError();
            return;
        }
        P_DEBUG_ASSERT(new_handle != nullptr && new_handle != INVALID_HANDLE_VALUE);

        /* Commit the newly created state onto this object. */
        m_ptr    = static_cast<u8 *>(ptr) + alignment;
        m_len    = len;
        m_handle = new_handle;
    }

    void MemoryMappedImpl::Unmap() {
        /* If we don't maintain an allocation, we have nothing to do. */
        if (this->IsMapped()) {
            return;
        }

        /* Compute the pointer alignment to revert applied offsets from mapping. */
        const size_t alignment = reinterpret_cast<usize>(m_ptr) % GetAllocationGranularity();

        /* Unmap the file view and close the stored handle. */
        ::UnmapViewOfFile(m_ptr - alignment);
        ::CloseHandle(m_handle);
    }

    void MemoryMappedImpl::Flush(size_t offset, size_t len, std::error_code &ec) {
        /* Attempt to flush the memory region asynchronously. */
        this->FlushAsync(offset, len, ec);

        /* If we got a handle, try to flush its file buffers. */
        if (!ec && m_handle != INVALID_HANDLE_VALUE) {
            if (::FlushFileBuffers(m_handle) == 0) {
                ec = GetLastOsError();
            }
        }
    }

    void MemoryMappedImpl::FlushAsync(size_t offset, size_t len, std::error_code &ec) {
        /* Reset the error code back into a successful state. */
        ec.clear();

        /* If we don't maintain an allocation, we have nothing to do. */
        if (this->IsMapped()) {
            return;
        }

        /* Flush the file view and ensure the operation succeeded. */
        if (::FlushViewOfFile(m_ptr + offset, len) == 0) {
            ec = GetLastOsError();
        }
    }

    u64 GetFileSize(HANDLE handle, std::error_code &ec) {
        /* Reset the error code back into a successful state. */
        ec.clear();

        /* Try to query file size information by handle. */
        BY_HANDLE_FILE_INFORMATION info;
        if (::GetFileInformationByHandle(handle, std::addressof(info)) == 0) {
            ec = GetLastOsError();
            return 0;
        }

        return JoinHalves(info.nFileSizeHigh, info.nFileSizeLow);
    }

}
