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

#include <climits>

/* Disables the copy constructor of a class. */
#define P_DISALLOW_COPY(__TYPE__) __TYPE__(const __TYPE__ &) = delete

/* Disables the copy assignment operator of a class. */
#define P_DISALLOW_ASSIGN(__TYPE__) __TYPE__ &operator=(const __TYPE__ &) = delete

/* Disables both copy constructor and copy assignment operator of a class. */
#define P_DISALLOW_COPY_AND_ASSIGN(__TYPE__) \
    P_DISALLOW_COPY(__TYPE__);               \
    P_DISALLOW_ASSIGN(__TYPE__)

/* Disables the move constructor and move assignment operator of a class. */
#define P_DISALLOW_MOVE(__TYPE__)             \
    __TYPE__(__TYPE__ &&) = delete;           \
    __TYPE__ *operator=(__TYPE__ &&) = delete

/* Textually concatenates two tokens. */
#define P_CONCAT_IMPL(s1, s2) s1##s2
#define P_CONCAT(s1, s2) P_CONCAT_IMPL(s1, s2)

/* Converts a given expression to a string literal. */
#define P_STRINGIZE_IMPL(s) #s
#define P_STRINGIZE(s) P_STRINGIZE_IMPL(s)

/* Creates an anonymous variable with a unique name. */
#ifdef __COUNTER__
    #define P_ANON_VAR(str) P_CONCAT(str, __COUNTER__)
#else
    #define P_ANON_VAR(str) P_CONCAT(str, __LINE__)
#endif

/* Computes the size of a type in bits. */
#define BITSIZEOF(x) (sizeof(x) * CHAR_BIT)

/* Common C++ attributes unified as macros. */
#define P_NODISCARD   [[nodiscard]]
#define P_NORETURN    [[noreturn]]
#define P_FALLTHROUGH [[fallthrough]]
#define P_LIKELY      [[likely]]
#define P_UNLIKELY    [[unlikely]]

#if defined(__GNUC__) || defined(__clang__)
    #define P_ALWAYS_INLINE        [[gnu::always_inline]] inline
    #define P_ALWAYS_INLINE_LAMBDA __attribute__((always_inline))
    #define P_NOINLINE             [[using gnu: cold, noinline]]

    #define P_BUILTIN_UNREACHABLE() __builtin_unreachable()
    #define P_ASSUME(expr)                     \
         do {                                  \
             if (!static_cast<bool>((expr))) { \
                 P_BUILTIN_UNREACHABLE();      \
             }                                 \
         } while (false)

#elif defined(_MSC_VER)
    #define P_ALWAYS_INLINE        [[msvc::always_inline]]
    #define P_ALWAYS_INLINE_LAMBDA
    #define P_NOINLINE             [[msvc::noinline]]

    #define P_BUILTIN_UNREACHABLE() __assume(0)
    #define P_ASSUME(expr)          __assume(expr)

#else
    #error "unsupported compiler; please use GCC, Clang or MSVC"
#endif

/* Discards unused variables without side effects. */
#define P_UNUSED(...) ::ptor::impl::UnusedImpl(__VA_ARGS__)

namespace ptor::impl {

    template <typename... Args>
    P_ALWAYS_INLINE constexpr void UnusedImpl(Args &&...args) {
        (static_cast<void>(args), ...);
    }

}
