/*
 * Copyright (c) 2021-2022 Valentin B.
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

#include <cstring>

#include "util/util_byteorder.hpp"

namespace ptor::util {

    /* Helpers for encoding and decoding integral types in different byte orders. */

    namespace impl {

        template <typename U> requires std::unsigned_integral<U>
        P_ALWAYS_INLINE void EncodeBigEndian(void *buf, const U value, size_t nbytes) {
            const u8 *value_bytes = reinterpret_cast<const u8 *>(std::addressof(value));
            std::memcpy(buf, value_bytes + (sizeof(U) - nbytes), nbytes);
        }

        template <typename U> requires std::unsigned_integral<U>
        P_ALWAYS_INLINE void EncodeLittleEndian(void *buf, const U value, size_t nbytes) {
            std::memcpy(buf, std::addressof(value), nbytes);
        }

        template <typename U> requires std::unsigned_integral<U>
        P_ALWAYS_INLINE void DecodeBigEndian(U *out, const void *buf, size_t nbytes) {
            u8 *out_bytes = reinterpret_cast<u8 *>(out);
            std::memcpy(out_bytes + (sizeof(U) - nbytes), buf, nbytes);
        }

        template <typename U> requires std::unsigned_integral<U>
        P_ALWAYS_INLINE void DecodeLittleEndian(U *out, const void *buf, size_t nbytes) {
            std::memcpy(out, buf, nbytes);
        }

        template <typename U> requires std::unsigned_integral<U>
        P_ALWAYS_INLINE constexpr auto ExtendSign(const U value, size_t nbytes) {
            using T = std::make_signed_t<U>;

            auto shift = (sizeof(U) - nbytes) * 8;
            return static_cast<T>(value << shift) >> shift;
        }

        template <typename T> requires std::signed_integral<T>
        P_ALWAYS_INLINE constexpr auto UnextendSign(const T value, size_t nbytes) {
            using U = std::make_unsigned_t<T>;

            auto shift = (sizeof(U) - nbytes) * 8;
            return (static_cast<U>(value) << shift) >> shift;
        }

    }

    template <typename U, std::endian BO> requires std::unsigned_integral<U>
    P_ALWAYS_INLINE void Encode(void *buf, const U value, size_t nbytes) {
        if constexpr (BO == std::endian::big) {
            impl::EncodeBigEndian(buf, ToBigEndian<U>(value), nbytes);

        } else {
            static_assert(BO == std::endian::little);
            impl::EncodeLittleEndian(buf, ToLittleEndian<U>(value), nbytes);
        }
    }

    template <typename U, std::endian BO> requires std::unsigned_integral<U>
    P_ALWAYS_INLINE void Encode(void *buf, const U value) {
        Encode<U, BO>(buf, value, sizeof(U));
    }

    template <typename T, std::endian BO> requires std::signed_integral<T>
    P_ALWAYS_INLINE void Encode(void *buf, const T value, size_t nbytes) {
        using U = std::make_unsigned_t<T>;
        Encode<U, BO>(buf, impl::UnextendSign<T>(value, nbytes), nbytes);
    }

    template <typename T, std::endian BO> requires std::signed_integral<T>
    P_ALWAYS_INLINE void Encode(void *buf, const T value) {
        Encode<T, BO>(buf, value, sizeof(T));
    }

    template <typename U, std::endian BO> requires std::unsigned_integral<U>
    P_ALWAYS_INLINE U Decode(const void *buf, size_t nbytes) {
        U out{0};
        if constexpr (BO == std::endian::big) {
            impl::DecodeBigEndian<U>(std::addressof(out), buf, nbytes);
            return ToBigEndian<U>(out);

        } else {
            static_assert(BO == std::endian::little);
            impl::DecodeLittleEndian<U>(std::addressof(out), buf, nbytes);
            return ToLittleEndian<U>(out);
        }
    }

    template <typename U, std::endian BO> requires std::unsigned_integral<U>
    P_ALWAYS_INLINE U Decode(const void *buf) {
        return Decode<U, BO>(buf, sizeof(U));
    }

    template <typename T, std::endian BO> requires std::signed_integral<T>
    P_ALWAYS_INLINE T Decode(const void *buf, size_t nbytes) {
        using U = std::make_unsigned_t<T>;
        return impl::ExtendSign<U>(Decode<U, BO>(buf, nbytes), nbytes);
    }

    template <typename T, std::endian BO> requires std::signed_integral<T>
    P_ALWAYS_INLINE T Decode(const void *buf) {
        return Decode<T, BO>(buf, sizeof(T));
    }

}
