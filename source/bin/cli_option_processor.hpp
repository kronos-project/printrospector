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

#include <type_traits>

#include "ptor_defines.hpp"
#include "bin/cli_options.hpp"
#include "util/util_i_function.hpp"

namespace ptor::cli {

    /* Wraps a command-line option and a callback used for parsing it into `Options`. */
    struct OptionProcessor {
    public:
        using CallbackType = util::IFunction<bool(Options &, const char *)>;

    private:
        /* TODO: Shrink storage down to what we really need. */
        u8 m_handler_storage[sizeof(usize) * 4]{};
        const CallbackType &m_callback;

    public:
        const char *name;
        char short_name;
        bool takes_arg;
        const char *short_help;
        const char *help;

    public:
        OptionProcessor(const char *n, char sn, const char *sh, const char *h, bool ta, auto f)
            : m_callback(*reinterpret_cast<const decltype(CallbackType::Make(f)) *>(m_handler_storage)),
              name(n), short_name(sn), takes_arg(ta), short_help(sh), help(h)
        {
            using FunctionType = decltype(CallbackType::Make(f));
            static_assert(sizeof(m_handler_storage) >= sizeof(FunctionType));
            std::construct_at(reinterpret_cast<FunctionType *>(m_handler_storage), CallbackType::Make(f));
        }

        P_ALWAYS_INLINE bool Parse(Options &opts, const char *str) const {
            return m_callback(opts, str);
        }
    };

    /* Creates a new `OptionProcessor` from option name and parser callback. */
    P_ALWAYS_INLINE OptionProcessor MakeProcessor(const char *n, char sn, const char *sh, const char *h, auto f) {
        if constexpr (requires { f(std::declval<Options &>(), std::declval<const char *>()); }) {
            if constexpr (std::convertible_to<decltype(f(std::declval<Options &>(), std::declval<const char *>())), bool>) {
                return OptionProcessor(n, sn, sh, h, true, f);
            } else {
                return OptionProcessor(n, sn, sh, h, true, [&](Options &opts, const char *str) -> bool { f(opts, str); return true; });
            }
        } else {
            if constexpr (std::convertible_to<decltype(f(std::declval<Options &>())), bool>) {
                return OptionProcessor(n, sn, sh, h, false, [&](Options &opts, const char *) -> bool { return f(opts); });
            } else {
                return OptionProcessor(n, sn, sh, h, false, [&](Options &opts, const char *) -> bool { f(opts); return true; });
            }
        }
    }

    P_ALWAYS_INLINE OptionProcessor MakeProcessor(const char *n, const char *sh, const char *h, auto f) {
        return MakeProcessor(n, '\0', sh, h, f);
    }

}
