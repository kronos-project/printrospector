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

#include <algorithm>
#include <string_view>
#include <type_traits>

#include "fmt/core.h"

#include "ptor_defines.hpp"

namespace ptor {

    /* Evaluates the supplied expression to a bool and crashes on `false`. */
    #define P_ASSERT(expr, ...) P_ASSERT_IMPL(expr, ##__VA_ARGS__)

    /* Unconditionally triggers an assertion failure at the current position. */
    /* This is equivalent to `P_ASSERT(false, ...)` and should be used for    */
    /* unpredictable program conditions that are impossible to recover from.  */
    #define P_ABORT(...) P_CALL_ASSERT_FAIL_IMPL(P_ASSERT_IMPL_SOURCE_LOCATION, "", __VA_ARGS__)

    /* An unreachable branch in the program's control flow. */
    /* Will terminate the program in an abort when hit.     */
    #define P_UNREACHABLE() P_CALL_ASSERT_FAIL_IMPL(P_ASSERT_IMPL_SOURCE_LOCATION, "", "entered unreachable code")

    /* Indicates unimplemented code by aborting execution.         */
    /* Use `P_TODO()` when intending to implement at a later time. */
    #define P_UNIMPLEMENTED() P_CALL_ASSERT_FAIL_IMPL(P_ASSERT_IMPL_SOURCE_LOCATION, "", "not implemented")

    /* Indicates yet unfinished code by aborting execution. */
    /* Useful for prototyping as it conveys the intent.     */
    #define P_TODO() P_CALL_ASSERT_FAIL_IMPL(P_ASSERT_IMPL_SOURCE_LOCATION, "", "not implemented")

    /* Same as `P_ASSERT` when building in debug configuration.                  */
    /* Expands to `P_ASSUME` in release builds without asserting the expression. */
    #if defined(P_ENABLE_DEBUG_ASSERTIONS)
        #define P_DEBUG_ASSERT(expr, ...) P_ASSERT_IMPL(expr, ##__VA_ARGS__)
    #else
        #define P_DEBUG_ASSERT(expr, ...) P_ASSUME(expr)
    #endif

    namespace impl {

        struct SourceLocation {
            const char *file;
            const char *func;
            unsigned line;
        };

        consteval const char *TrimSourcePath(std::string_view source) {
            const auto rfind = [source](const std::string_view match) {
                if (source.rfind(match) == std::string_view::npos) {
                    return std::string_view::size_type{0};
                } else {
                    return source.rfind(match) + match.size();
                }
            };

            auto idx = std::max({rfind("source/"), rfind("source\\"), rfind("../"), rfind("..\\")});
            return source.data() + idx;
        }

        inline void CrashAtConstEvaluation(const char *) { /* ... */ }

        #define P_ASSERT_IMPL_SOURCE_LOCATION           \
            ::ptor::impl::SourceLocation {              \
                ::ptor::impl::TrimSourcePath(__FILE__), \
                __func__,                               \
                static_cast<unsigned>(__LINE__)         \
            }

        #define P_ASSERT_IMPL(expr, ...)                                                          \
            [&, __ptor_assert_src_loc = P_ASSERT_IMPL_SOURCE_LOCATION]() P_ALWAYS_INLINE_LAMBDA { \
                if (const bool __expr = static_cast<bool>((expr)); !__expr) P_UNLIKELY {          \
                    P_CALL_ASSERT_FAIL_IMPL(__ptor_assert_src_loc, #expr, ##__VA_ARGS__);         \
                }                                                                                 \
            }()

        #define P_CALL_ASSERT_FAIL_IMPL(source, expr, ...)                                      \
            [&](const ::ptor::impl::SourceLocation &loc) P_ALWAYS_INLINE_LAMBDA {               \
                if (std::is_constant_evaluated()) {                                             \
                    ::ptor::impl::CrashAtConstEvaluation("assertion failed at compile time");   \
                } else {                                                                        \
                    ::ptor::impl::AssertFailImpl(loc, expr, ##__VA_ARGS__);                     \
                }                                                                               \
            }(source)

        template <typename... Args>
        P_NORETURN P_NOINLINE void AssertFailImpl(const SourceLocation &loc, const char *expr, std::string_view format, const Args... args) {
            ReifiedAssertFailImpl(loc, expr, format, fmt::make_format_args(args...));
        }

        P_NORETURN P_NOINLINE void AssertFailImpl(const SourceLocation &loc, const char *expr);

        P_NORETURN P_NOINLINE void ReifiedAssertFailImpl(const SourceLocation &src, const char *expr, std::string_view format, fmt::format_args args);

    }

}
