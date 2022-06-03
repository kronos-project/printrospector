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

#include <memory>

#include "ptor_defines.hpp"

namespace ptor::util {

    namespace impl {

        struct ExitToken {};

        template <class Fn>
        class ScopeGuard final {
            P_DISALLOW_COPY(ScopeGuard);

        private:
            Fn m_fn;
            bool m_active;

        public:
            P_ALWAYS_INLINE constexpr explicit ScopeGuard(Fn fn) : m_fn{std::move(fn)}, m_active{true} {}

            P_ALWAYS_INLINE constexpr ~ScopeGuard() {
                if (m_active) {
                    m_fn();
                }
            }

            P_ALWAYS_INLINE constexpr void Cancel() { m_active = false; }

            P_ALWAYS_INLINE constexpr ScopeGuard(ScopeGuard &&rhs) : m_fn{std::move(rhs.m_fn)}, m_active{rhs.m_active} {
                rhs.Cancel();
            }

            ScopeGuard &operator=(ScopeGuard &&) = delete;
        };

        /* `->*` has the highest precedence of operators we can use. */
        template <class Fn>
        P_ALWAYS_INLINE static constexpr ScopeGuard<Fn> operator->*(ExitToken, Fn &&fn) {
            return ScopeGuard<Fn>(std::forward<Fn>(fn));
        }

    }

    /* Public-facing helper macros for scope guard construction. */
    #define P_MAKE_SCOPE_GUARD ::ptor::util::impl::ExitToken{}->*[&]() P_ALWAYS_INLINE_LAMBDA
    #define P_ON_SCOPE_EXIT auto P_ANON_VAR(__PTOR_SCOPE_EXIT_GUARD__) = P_MAKE_SCOPE_GUARD

}
