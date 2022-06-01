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

#include <concepts>
#include <type_traits>

#include "assert.hpp"
#include "ptor_types.hpp"

namespace ptor::util {

    namespace impl {

        template <std::integral T>
        P_ALWAYS_INLINE constexpr bool IsPowerOfTwo(T value) {
            return value > 0 && (value & (value - 1)) == 0;
        }

    }

    template <typename T>
    P_ALWAYS_INLINE constexpr T AlignUp(T value, size_t align) {
        using U = std::make_unsigned_t<T>;

        P_DEBUG_ASSERT(impl::IsPowerOfTwo(align));
        const U mask = static_cast<U>(align - 1);
        return static_cast<T>((value + mask) & ~mask);
    }

    template <typename T>
    P_ALWAYS_INLINE constexpr T AlignDown(T value, size_t align) {
        using U = std::make_unsigned_t<T>;

        P_DEBUG_ASSERT(impl::IsPowerOfTwo(align));
        const U mask = static_cast<U>(align - 1);
        return static_cast<T>(value & ~mask);
    }

    template <typename T>
    P_ALWAYS_INLINE constexpr bool IsAligned(T value, size_t align) {
        using U = std::make_unsigned_t<T>;

        P_DEBUG_ASSERT(impl::IsPowerOfTwo(align));
        const U mask = static_cast<U>(align - 1);
        return (value & mask) == 0;
    }

    template <>
    P_ALWAYS_INLINE void *AlignUp<void *>(void *value, size_t align) {
        return reinterpret_cast<void *>(AlignUp(reinterpret_cast<usize>(value), align));
    }

    template <>
    P_ALWAYS_INLINE const void *AlignUp<const void *>(const void *value, size_t align) {
        return reinterpret_cast<const void *>(AlignUp(reinterpret_cast<usize>(value), align));
    }

    template <>
    P_ALWAYS_INLINE void *AlignDown<void *>(void *value, size_t align) {
        return reinterpret_cast<void *>(AlignDown(reinterpret_cast<usize>(value), align));
    }

    template <>
    P_ALWAYS_INLINE const void *AlignDown<const void *>(const void *value, size_t align) {
        return reinterpret_cast<const void *>(AlignDown(reinterpret_cast<usize>(value), align));
    }

    template <>
    P_ALWAYS_INLINE bool IsAligned<void *>(void *value, size_t align) {
        return IsAligned(reinterpret_cast<usize>(value), align);
    }

    template <>
    P_ALWAYS_INLINE bool IsAligned<const void *>(const void *value, size_t align) {
        return IsAligned(reinterpret_cast<usize>(value), align);
    }

}
