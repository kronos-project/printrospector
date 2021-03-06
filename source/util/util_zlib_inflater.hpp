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

#include <system_error>

#include "libdeflate.h"

#include "ptor_defines.hpp"
#include "ptor_types.hpp"
#include "util/util_literals.hpp"

namespace ptor::util {

    /* A class that handles zlib decompression of data. */
    /* After calling `Decompress()` successfully, one may assume `GetPtr()` returns a   */
    /* valid pointer to the decompressed data until another `Decompress()` takes place. */
    class Inflater {
        P_DISALLOW_COPY_AND_ASSIGN(Inflater);

    public:
        static constexpr size_t DefaultCapacity = 64_MB;

    private:
        libdeflate_decompressor *m_decompressor;
        u8 *m_buffer;
        size_t m_size;

    private:
        P_ALWAYS_INLINE Inflater() : m_decompressor{nullptr}, m_buffer{nullptr}, m_size{0} {}

        Inflater(libdeflate_decompressor *d, u8 *buf, size_t size);

    public:
        Inflater(Inflater &&rhs);

        Inflater &operator=(Inflater &&rhs);

        ~Inflater();

        static Inflater Allocate(std::error_code &ec);

        P_ALWAYS_INLINE u8 *GetCurrentBufferPtr() { return m_buffer; }
        P_ALWAYS_INLINE const u8 *GetCurrentBufferPtr() const { return m_buffer; }

    private:
        void Grow(size_t new_size, std::error_code &ec);

        size_t DecompressImpl(const void *data, size_t len, std::error_code &ec);

    public:
        size_t Decompress(const void *data, size_t len, size_t size_hint, std::error_code &ec);
    };

}
