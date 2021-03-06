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

#include <cstdint>
#include <filesystem>
#include <limits>

namespace ptor {

    namespace fs = std::filesystem;

}

using u8    = std::uint8_t;
using u16   = std::uint16_t;
using u32   = std::uint32_t;
using u64   = std::uint64_t;
using usize = std::uintptr_t;

using i8    = std::int8_t;
using i16   = std::int16_t;
using i32   = std::int32_t;
using i64   = std::int64_t;
using isize = std::intptr_t;

using f32 = float;
using f64 = double;

static_assert(sizeof(f32) == sizeof(u32) && std::numeric_limits<f32>::is_iec559);
static_assert(sizeof(f64) == sizeof(u64) && std::numeric_limits<f64>::is_iec559);

using size_t    = std::size_t;
using ptrdiff_t = std::ptrdiff_t;

#define P_BIT(n) (1u << (n))

#define P_BITL(n) (1ul << (n))

#define P_BITLL(n) (1ull << (n))

#define P_LSB(n) (P_BIT(n) - 1u)

#define P_LSBL(n) (P_BITL(n) - 1ul)

#define P_LSBLL(n) (P_BITLL(n) - 1ull)

#define P_MASK(start, end) (P_LSB(end) & ~P_LSB(start))

#define P_MASKL(start, end) (P_LSBL(end) & ~P_LSBL(start))

#define P_MASKLL(start, end) (P_LSBLL(end) & ~P_LSBLL(start))
