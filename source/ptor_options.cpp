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

#include <cstring>
#include <utility>

#include "ptor_options.hpp"
#include "util/util_i_function.hpp"

namespace ptor {

    namespace {

        /* Wraps a command-line option and a callback used for parsing it into `Options`. */
        struct OptionProcessor {
        public:
            using CallbackType = util::IFunction<bool(Options &, const char *)>;

        private:
            u8 m_handler_storage[sizeof(usize) * 4]{};
            const CallbackType &m_callback;

        public:
            const char *name;
            char short_name;
            bool takes_arg;

        public:
            OptionProcessor(const char *n, char sn, bool ta, auto f)
                : m_callback(*reinterpret_cast<const decltype(CallbackType::Make(f)) *>(m_handler_storage)),
                  name(n), short_name(sn), takes_arg(ta)
            {
                using FunctionType = decltype(CallbackType::Make(f));
                static_assert(sizeof(m_handler_storage) >= sizeof(FunctionType));
                std::construct_at(reinterpret_cast<FunctionType *>(m_handler_storage), CallbackType::Make(f));
            }

            P_ALWAYS_INLINE bool Parse(Options &options, const char *str) const {
                return m_callback(options, str);
            }
        };

        /* Creates a new `OptionProcessor` from option name and parsing callback. */
        OptionProcessor MakeProcessor(const char *n, char sn, auto f) {
            if constexpr (requires { f(std::declval<Options &>(), std::declval<const char *>()); }) {
                if constexpr (std::convertible_to<decltype(f(std::declval<Options &>(), std::declval<const char *>())), bool>) {
                    return OptionProcessor(n, sn, true, f);
                } else {
                    return OptionProcessor(n, sn, true, [&](Options &opts, const char *str) -> bool { f(opts, str); return true; });
                }
            } else {
                if constexpr (std::convertible_to<decltype(f(std::declval<Options &>())), bool>) {
                    return OptionProcessor(n, sn, false, [&](Options &opts, const char *) -> bool { return f(opts); });
                } else {
                    return OptionProcessor(n, sn, false, [&](Options &opts, const char *) -> bool { f(opts); return true; });
                }
            }
        }

        /* A list of `OptionProcessor`s for all command-line options we accept. */
        const OptionProcessor g_option_processors[] = {};

        P_ALWAYS_INLINE constexpr bool IsShortOption(const char *arg, size_t arg_len) {
            return arg_len >= 2 && arg[0] == '-' && arg[1] != '-';
        }

        P_ALWAYS_INLINE constexpr bool IsLongOption(const char *arg, size_t arg_len) {
            return arg_len >= 2 && arg[0] == '-' && arg[1] == '-';
        }

        bool ParseOption(int argc, char **argv, Options &opts, const OptionProcessor &opt, const char *arg, size_t len, int &idx) {
            if (opt.takes_arg) {
                if (arg[2 + len] == '=') {
                    return opt.Parse(opts, arg + 2 + len + 1);
                } else {
                    ++idx;
                    return idx < argc && opt.Parse(opts, argv[idx]);
                }
            } else {
                return opt.Parse(opts, nullptr);
            }
        }

    }

    std::optional<Options> ParseOptionsFromArgs(int argc, char **argv) {
        /* Create a default instance of the command line options. */
        Options options{};

        /* Iterate over each command-line option and try to parse it. */
        for (int i = 1; i < argc; ++i) {
            /* Get the current argument and its length. */
            const auto *arg    = argv[i];
            const auto arg_len = std::strlen(arg);

            /* Try to parse the current argument. */
            bool valid = false;
            if (IsShortOption(arg, arg_len)) {
                /* We're dealing with an `-a`-style option here. */
                for (const auto &opt : g_option_processors) {
                    if (arg[1] != opt.short_name) {
                        continue;
                    }

                    valid = ParseOption(argc, argv, options, opt, arg, 0, i);

                    break;
                }

            } else if (IsLongOption(arg, arg_len)) {
                /* We're dealing with an `--arg`-style option here. */
                for (const auto &opt : g_option_processors) {
                    const auto name_len = std::strlen(opt.name);
                    if (arg_len < name_len + 2 || std::memcmp(arg + 2, opt.name, name_len) != 0 || (arg[2 + name_len] != '\0' && arg[2 + name_len] != '=')) {
                        continue;
                    }

                    valid = ParseOption(argc, argv, options, opt, arg, name_len, i);

                    break;
                }
            }

            /* Check if we succeeded at parsing the current argument or */
            /* print usage details for the user instead.                */
            if (!valid) {
                /* TODO: Print usage info. */
                return {};
            }
        }

        /* TODO: Check for missing required arguments. */

        /* We made it through successfully, return the parsed options. */
        return options;
    }

}
