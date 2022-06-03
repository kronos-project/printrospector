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

#include "io/impl/io_memory_mapped.unix.hpp"

#include <algorithm>
#include <cerrno>
#include <system_error>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util/util_alignment.hpp"

namespace ptor::io::impl {

    namespace {

        static const size_t PageSize = sysconf(_SC_PAGE_SIZE);

        P_ALWAYS_INLINE std::error_code GetLastOsError() {
            return {errno, std::system_category()};
        }

    }

    MemoryMappedImpl::~MemoryMappedImpl() {
        this->Unmap();
    }

    MemoryMappedImpl::MemoryMappedImpl(MemoryMappedImpl &&rhs) : m_ptr{rhs.m_ptr}, m_len{rhs.m_len} {
        /* Reset rhs back into the default object state. */
        rhs.m_ptr = nullptr;
        rhs.m_len = 0;
    }

    MemoryMappedImpl &MemoryMappedImpl::operator=(MemoryMappedImpl &&rhs) {
        /* First, remove the existing mapping from the target. */
        this->Unmap();

        /* Move the state in rhs over. */
        m_ptr = rhs.m_ptr;
        m_len = rhs.m_len;

        /* Reset rhs back into the default object state. */
        rhs.m_ptr = nullptr;
        rhs.m_len = 0;

        return *this;
    }

    void MemoryMappedImpl::Map(int fd, int protect, int flags, size_t offset, size_t len, std::error_code &ec) {
        /* Reset the error code back into a successful state. */
        ec.clear();

        /* Compute offset and length with respect to page alignment. */
        const size_t aligned_offset = util::AlignDown(offset, PageSize);
        const size_t alignment      = offset - aligned_offset;
        const size_t aligned_len    = len + alignment;

        /* Explicitly check for len 0 as we're not allowed to allocate that. */
        if (aligned_len == 0) {
            return;
        }

        /* Memory-map the file view and ensure the operation succeeded. */
        auto *ptr = mmap(nullptr, aligned_len, protect, flags, fd, aligned_offset);
        if (ptr == MAP_FAILED) {
            ec = GetLastOsError();
            return;
        }

        /* Commit the newly created state onto this object. */
        m_ptr = static_cast<u8 *>(ptr) + alignment;
        m_len = len;
    }

    void MemoryMappedImpl::Unmap() {
        /* If we don't maintain an allocation, we have nothing to do. */
        if (this->IsMapped()) {
            return;
        }

        /* Compute the pointer alignment to revert applied offsets from mapping. */
        const size_t alignment = reinterpret_cast<usize>(m_ptr) % PageSize;

        /* Unmap the file view. */
        munmap(m_ptr - alignment, m_len + alignment);
    }

    void MemoryMappedImpl::Flush(size_t offset, size_t len, std::error_code &ec) {
        /* Reset the error code back into a successful state. */
        ec.clear();

        /* If we don't maintain an allocation, we have nothing to do. */
        if (this->IsMapped()) {
            return;
        }

        /* Compute offset and length with respect to page alignment. */
        const size_t alignment         = (reinterpret_cast<usize>(m_ptr) + offset) % PageSize;
        const ptrdiff_t aligned_offset = offset - alignment;
        const size_t aligned_len       = len + alignment;

        /* Try to flush the memory region. */
        if (msync(m_ptr + aligned_offset, aligned_len, MS_SYNC) != 0) {
            ec = GetLastOsError();
        }
    }

    void MemoryMappedImpl::FlushAsync(size_t offset, size_t len, std::error_code &ec) {
        /* Reset the error code back into a successful state. */
        ec.clear();

        /* If we don't maintain an allocation, we have nothing to do. */
        if (this->IsMapped()) {
            return;
        }

        /* Compute offset and length with respect to page alignment. */
        const size_t alignment         = (reinterpret_cast<usize>(m_ptr) + offset) % PageSize;
        const ptrdiff_t aligned_offset = offset - alignment;
        const size_t aligned_len       = len + alignment;

        /* Try to flush the memory region. */
        if (msync(m_ptr + aligned_offset, aligned_len, MS_ASYNC) != 0) {
            ec = GetLastOsError();
        }
    }

    u64 GetFileSize(int fd, std::error_code &ec) {
    #if defined(PTOR_OS_LINUX) || defined(__EMSCRIPTEN__)
        #define P_FSTAT fstat64
        #define P_STAT  struct stat64
    #else
        #define P_FSTAT fstat
        #define P_STAT  struct stat
    #endif

    /* Reset the error code back into a successful state. */
    ec.clear();

    /* Try to query file size information by file descriptor. */
    P_STAT sb;
    if (P_FSTAT(fd, std::addressof(sb)) == 0) {
        return sb.st_size;
    } else {
        ec = GetLastOsError();
        return 0;
    }

    #undef P_FSTAT
    #undef P_STAT
    }

}
