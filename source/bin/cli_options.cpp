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

        inline u32 IntParseHelper(const char *str, bool &success) {
            u32 value;
            {
                char *end;
                errno = 0;
                value = std::strtoul(str, &end, 0);
                success = (str != end && errno == 0);
            }
            return value;
        }

        /* Global list of `OptionProcessor`s for all command-line options we accept. */
        const OptionProcessor g_option_processors[] = {
            MakeProcessor("serialize-opt", 'd', [](Options &opts, const char *value) {
                if (std::strcmp(value, "ser") == 0) {
                    opts.serialize_opt = SerializeOpt::Serialize;
                } else if (std::strcmp(value, "de") == 0) {
                    opts.serialize_opt = SerializeOpt::Deserialize;
                } else {
                    return false;
                }

                return true;
            }),
            MakeProcessor("data-kind", 'k', [](Options &opts, const char *value) {
                if (std::strcmp(value, "op") == 0) {
                    opts.data_kind = DataKind::ObjectProperty;
                } else {
                    return false;
                }

                return true;
            }),
            MakeProcessor("hex", [](Options &opts, const char *value) {
                opts.input_type = InputType::Hex;
                opts.input_hex = value;
            }),
            MakeProcessor("infile", 'i', [](Options &opts, const char *value) {
                opts.input_type = InputType::File;
                opts.input_file = value;
                return opts.input_file.has_filename();
            }),
            MakeProcessor("out", 'o', [](Options &opts, const char *value) {
                opts.output = value;
                return opts.output.has_filename();
            }),
            MakeProcessor("type-list", 't', [](Options &opts, const char *value) {
                opts.type_list = value;
                return opts.type_list.has_filename();
            }),
            MakeProcessor("serializer-type", 's', [](Options &opts, const char *value) {
                if (std::strcmp(value, "basic") == 0) {
                    opts.serializer_type = SerializerType::Basic;
                } else if (std::strcmp(value, "core") == 0) {
                    opts.serializer_type = SerializerType::CoreObject;
                } else if (std::strcmp(value, "mannequin") == 0) {
                    opts.serializer_type = SerializerType::Mannequin;
                } else {
                    return false;
                }

                return true;
            }),
            MakeProcessor("serializer-flags", 'f', [](Options &opts, const char *value) {
                bool result = false;
                opts.serializer_flags = IntParseHelper(value, result) & 0x1F;
                return result;
            }),
            MakeProcessor("property-mask", 'm', [](Options &opts, const char *value) {
                bool success = false;
                opts.property_mask = IntParseHelper(value, success);
                return success;
            }),
            MakeProcessor("shallow", [](Options &opts) { opts.shallow = true; }),
            MakeProcessor("manual-compression", 'c', [](Options &opts) { opts.manual_compression = true; }),
            MakeProcessor("quiet", 'q', [](Options &opts) { opts.quiet = true; }),
            MakeProcessor("skip-unknown", [](Options &opts) { opts.skip_unknown = true; })
        };

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

        /* We're valid when there's at least any input source. */
        if (options.input_type == InputType::Unknown) {
            /* TODO: Print usage info. */
            return {};
        }

        return options;
    }

}
