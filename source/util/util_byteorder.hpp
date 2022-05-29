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

#include <bit>
#include <concepts>
#include <type_traits>

#include "ptor_defines.hpp"
#include "ptor_types.hpp"

#if defined(P_COMPILER_CLANG) || defined(P_COMPILER_GCC)
    #define P_BSWAP16(x) __builtin_bswap16(x)
    #define P_BSWAP32(x) __builtin_bswap32(x)
    #define P_BSWAP64(x) __builtin_bswap64(x)

#elif defined(P_COMPILER_MSVC)
    #include <stdlib.h>

    #define P_BSWAP16(x) ::ptor::util::impl::BSwap16(x)
    #define P_BSWAP32(x) ::ptor::util::impl::BSwap32(x)
    #define P_BSWAP64(x) ::ptor::util::impl::BSwap64(x)
#else
    #error "unsupported compiler; please use GCC, Clang or MSVC"
#endif

namespace ptor::util {

    #ifdef P_COMPILER_MSVC

    /* The MSVC byteswap intrinsics are not constexpr. */
    /* We still prefer them where applicable though.   */

    namespace impl {

        P_ALWAYS_INLINE constexpr u16 BSwap16(const u16 value) {
            if (std::is_constant_evaluated()) {
                constexpr u16 ByteMask = UCHAR_MAX;

                return ((value & (ByteMask << 8)) >> 8) |
                       ((value & (ByteMask << 0)) << 8);
            } else {
                static_assert(sizeof(unsigned short) == sizeof(u16));
                return _byteswap_ushort(value);
            }
        }

        P_ALWAYS_INLINE constexpr u32 BSwap32(const u32 value) {
            if (std::is_constant_evaluated()) {
                constexpr u32 ByteMask = UCHAR_MAX;

                return ((value & (ByteMask << 24)) >> 24) |
                       ((value & (ByteMask << 16)) >>  8) |
                       ((value & (ByteMask <<  8)) <<  8) |
                       ((value & (ByteMask <<  0)) << 24);
            } else {
                static_assert(sizeof(unsigned long) == sizeof(u32));
                return _byteswap_ulong(value);
            }
        }

        P_ALWAYS_INLINE constexpr u64 BSwap64(const u64 value) {
            if (std::is_constant_evaluated()) {
                constexpr u64 ByteMask = UCHAR_MAX;

                return ((value & (ByteMask << 56)) >> 56) |
                       ((value & (ByteMask << 48)) >> 40) |
                       ((value & (ByteMask << 40)) >> 24) |
                       ((value & (ByteMask << 32)) >>  8) |
                       ((value & (ByteMask << 24)) <<  8) |
                       ((value & (ByteMask << 16)) << 24) |
                       ((value & (ByteMask <<  8)) << 40) |
                       ((value & (ByteMask <<  0)) << 56);
            } else {
                static_assert(sizeof(unsigned __int64) == sizeof(u64));
                return _byteswap_uint64(value);
            }
        }

    }

    #endif

    consteval bool IsLittleEndian() {
        return std::endian::native == std::endian::little;
    }

    consteval bool IsBigEndian() {
        return std::endian::native == std::endian::big;
    }

    static_assert(IsLittleEndian() ^ IsBigEndian());

    template <typename U> requires std::unsigned_integral<U>
    P_ALWAYS_INLINE constexpr U SwapBytes(const U u) {
        if constexpr (std::is_same_v<U, u64>) {
            return P_BSWAP64(u);
        } else if constexpr (std::is_same_v<U, u32>) {
            return P_BSWAP32(u);
        } else if constexpr (std::is_same_v<U, u16>) {
            return P_BSWAP16(u);
        } else if constexpr (std::is_same_v<U, u8>) {
            return u;
        } else {
            static_assert(!std::is_same_v<U, U>, "Incompatible type requested for SwapBytes!");
        }
    }

    template <typename T> requires std::integral<T>
    P_ALWAYS_INLINE constexpr void SwapBytes(T *ptr) {
        using U = std::make_unsigned_t<T>;
        *ptr = static_cast<T>(SwapBytes<U>(static_cast<U>(*ptr)));
    }

    template <typename T> requires std::integral<T>
    P_ALWAYS_INLINE constexpr T ToBigEndian(const T value) {
        using U = std::make_unsigned_t<T>;

        if constexpr (IsBigEndian()) {
            return static_cast<T>(static_cast<U>(value));
        } else {
            return static_cast<T>(SwapBytes<U>(static_cast<U>(value)));
        }
    }

    template <typename T> requires std::integral<T>
    P_ALWAYS_INLINE constexpr T ToLittleEndian(const T value) {
        using U = std::make_unsigned_t<T>;

        if constexpr (IsBigEndian()) {
            return static_cast<T>(SwapBytes<U>(static_cast<U>(value)));
        } else {
            return static_cast<T>(static_cast<U>(value));
        }
    }

    template <typename T> requires std::integral<T>
    P_ALWAYS_INLINE constexpr T ReadBigEndian(const T *ptr) {
        return ToBigEndian<T>(*ptr);
    }

    template <typename T> requires std::integral<T>
    P_ALWAYS_INLINE constexpr T ReadLittleEndian(const T *ptr) {
        return ToLittleEndian<T>(*ptr);
    }

    template <typename T> requires std::integral<T>
    P_ALWAYS_INLINE constexpr void WriteBigEndian(T *ptr, const T value) {
        *ptr = ToBigEndian<T>(value);
    }

    template <typename T> requires std::integral<T>
    P_ALWAYS_INLINE constexpr void WriteLittleEndian(T *ptr, const T value) {
        *ptr = ToLittleEndian<T>(value);
    }

}

#undef P_BSWAP16
#undef P_BSWAP32
#undef P_BSWAP64
