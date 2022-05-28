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

#include "bin/cli_options.hpp"

#include <cstring>

#include "bin/cli_option_processor.hpp"

namespace ptor::cli {

    namespace {

        /* Global list of `OptionProcessor`s for all command-line options we accept. */
        const OptionProcessor g_option_processors[] = {};

        /* Implementation details of option parsing code. */

        P_ALWAYS_INLINE constexpr bool IsShortOption(const char *arg, size_t arg_len) {
            return arg_len >= 2 && arg[0] == '-' && arg[1] != '-';
        }

        P_ALWAYS_INLINE constexpr bool IsLongOption(const char *arg, size_t arg_len) {
            return arg_len >= 2 && arg[0] == '-' && arg[1] == '-';
        }

        bool ParseOption(int argc, char **argv, Options &opts, const OptionProcessor &opt, const char *arg, int &idx) {
            if (opt.takes_arg) {
                if (arg[0] == '=') {
                    return opt.Parse(opts, arg + 1);
                } else {
                    ++idx;
                    return idx < argc && opt.Parse(opts, argv[idx]);
                }
            } else {
                return opt.Parse(opts, nullptr);
            }
        }

        bool ParseShortOption(int argc, char **argv, Options &opts, const char *arg, int &idx) {
            /* Walk through all the available options. */
            for (const auto &opt : g_option_processors) {
                /* Check preconditions. */
                if (arg[0] != opt.short_name) {
                    continue;
                }

                /* If we found a matching processor, attempt to parse the arg. */
                return ParseOption(argc, argv, opts, opt, arg + 1, idx);
            }

            return false;
        }

        bool ParseLongOption(int argc, char **argv, Options &opts, const char *arg, size_t arg_len, int &idx) {
            /* Walk through all the available options. */
            for (const auto &opt : g_option_processors) {
                /* Check preconditions. */
                const auto name_len = std::strlen(opt.name);
                if (arg_len < name_len || std::memcmp(arg, opt.name, name_len) != 0 || (arg[name_len] != '\0' && arg[name_len] != '=')) {
                    continue;
                }

                /* If we found a matching processor, attempt to parse the arg. */
                return ParseOption(argc, argv, opts, opt, arg + name_len, idx);
            }

            return false;
        }

    }

    std::optional<Options> ParseOptionsFromArgs(int argc, char **argv) {
        /* Create a default instance of command line options. */
        Options options{};

        /* Iterate over each command-line argument and try to parse it. */
        for (int i = 1; i < argc; ++i) {
            const auto *arg    = argv[i];
            const auto arg_len = std::strlen(arg);

            bool parsed = false;
            if (IsShortOption(arg, arg_len)) {
                parsed = ParseShortOption(argc, argv, options, arg + 1, i);
            } else if (IsLongOption(arg, arg_len)) {
                parsed = ParseLongOption(argc, argv, options, arg + 2, arg_len - 2, i);
            }

            /* Check if we succeeded at parsing the current argument. */
            if (!parsed) {
                /* TODO: Print usage info. */
                return {};
            }
        }

        /* TODO: Check for missing required arguments. */

        return options;
    }

}
