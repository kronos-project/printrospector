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

#include "util/util_zlib_inflater.hpp"

#include <new>

#include "assert.hpp"

namespace ptor::util {

    Inflater::Inflater(libdeflate_decompressor *d, u8 *buf, size_t size)
        : m_decompressor{d}, m_buffer{buf}, m_size{size}
    {
        P_ASSERT(m_decompressor != nullptr && m_buffer != nullptr);
    }

    Inflater::Inflater(Inflater &&rhs)
        : m_decompressor{rhs.m_decompressor}, m_buffer{rhs.m_buffer}, m_size{rhs.m_size}
    {
        rhs.m_decompressor = nullptr;
        rhs.m_buffer       = nullptr;
        rhs.m_size         = 0;
    }

    Inflater &Inflater::operator=(Inflater &&rhs) {
        /* Move the decompressor state over. */
        m_decompressor = rhs.m_decompressor;
        m_buffer       = rhs.m_buffer;
        m_size         = rhs.m_size;

        /* Invalidate the other decompressor's state. */
        rhs.m_decompressor = nullptr;
        rhs.m_buffer       = nullptr;
        rhs.m_size         = 0;

        return *this;
    }

    Inflater::~Inflater() {
        libdeflate_free_decompressor(m_decompressor);
        free(m_buffer);
    }

    Inflater Inflater::Allocate(std::error_code &ec) {
        auto *d   = libdeflate_alloc_decompressor();
        auto *buf = std::malloc(DefaultCapacity);

        if (d == nullptr || buf == nullptr) {
            ec = std::make_error_code(std::errc::not_enough_memory);
            return {};
        }

        return Inflater{d, static_cast<u8 *>(buf), DefaultCapacity};
    }

    void Inflater::Grow(size_t new_size, std::error_code &ec) {
        /* Reset the error code back into a successful state. */
        ec.clear();

        if (new_size > m_size) {
            /* Allocate a new buffer and free the old one. */
            auto *new_buf = std::malloc(new_size);
            if (new_buf == nullptr) {
                ec = std::make_error_code(std::errc::not_enough_memory);
                return;
            }
            free(m_buffer);

            /* Set the new buffer state. */
            m_buffer = static_cast<u8 *>(new_buf);
            m_size   = new_size;
        }
    }

    size_t Inflater::DecompressImpl(const void *data, size_t len, std::error_code &ec) {
        /* Reset the error code back into a successful state. */
        ec.clear();

        size_t written;
        switch (libdeflate_zlib_decompress(m_decompressor, data, len, m_buffer, m_size, std::addressof(written))) {
            case LIBDEFLATE_SUCCESS:
                /* We succeeded. Return the actual written bytes. */
                return written;
            case LIBDEFLATE_BAD_DATA:
                /* The user provided invalid data, return the error. */
                ec = std::make_error_code(std::errc::illegal_byte_sequence);
                break;
            case LIBDEFLATE_INSUFFICIENT_SPACE:
                /* We don't have anough memory to complete the operation right now. */
                ec = std::make_error_code(std::errc::not_enough_memory);
                break;
            case LIBDEFLATE_SHORT_OUTPUT:
                P_UNREACHABLE();
        }

        return 0;
    }

    size_t Inflater::Decompress(const void *data, size_t len, size_t size_hint, std::error_code &ec) {
        /* Grow the buffer to the size of the given hint, if necessary. */
        if (this->Grow(size_hint, ec); ec) {
            return 0;
        }

        /* Attempt to decompress the supplied data. */
        /* TODO: Should we compensate for wrong size hints? */
        size_t written = this->DecompressImpl(data, len, ec);

        return written;
    }

}
