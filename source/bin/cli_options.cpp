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

#include <cstdlib>
#include <string>

#include <fmt/color.h>
#include <fmt/core.h>

#include "ptor_version.hpp"
#include "bin/cli_option_processor.hpp"

namespace ptor::cli {

    namespace {

        inline u32 IntParseHelper(const char *str, bool &success) {
            u32 value;
            {
                char *end;
                errno = 0;
                value = std::strtoul(str, std::addressof(end), 0);
                success = (str != end && *end == '\0' && errno == 0);
            }
            return value;
        }

        P_NORETURN void HelpCommandImpl(Options &opts, const char *value);

        /* Global list of `OptionProcessor`s for all command-line options we accept. */
        const OptionProcessor g_option_processors[] = {
            MakeProcessor(
                "help", 'h', "prints help for all options or a specific one",
                "",
                [](Options &opts, const char *value) { HelpCommandImpl(opts, value); }
            ),
            MakeProcessor(
                "serialize-opt", 'd', "choose between deserialization (default) and serialization",
                "printrospector lets you choose between whether the input source should be"
                "interpreted as data to serialize or as data to deserialize.\n\n"
                "Supported values to this option are:\n\n"
                "    - ser: Serializes the input data.\n"
                "    - de:  Deserializes the input data.",
                [](Options &opts, const char *value) {
                    if (std::strcmp(value, "ser") == 0) {
                        opts.serialize_opt = SerializeOpt::Serialize;
                    } else if (std::strcmp(value, "de") == 0) {
                        opts.serialize_opt = SerializeOpt::Deserialize;
                    } else {
                        return false;
                    }

                    return true;
                }
            ),
            MakeProcessor(
                "data-kind", 'k', "the type of data to process; defaults to op",
                "printrospector supports processing data in various different formats:\n\n"
                "    op - Serialization and deserialization of ObjectProperty binary state. [default]\n\n"
                "Specified inputs should be in the correct format.",
                [](Options &opts, const char *value) {
                    if (std::strcmp(value, "op") == 0) {
                        opts.data_kind = DataKind::ObjectProperty;
                    } else {
                        return false;
                    }

                    return true;
                }
            ),
            MakeProcessor(
                "hex", "specifies a string of hexadecimal data as an input source",
                "As an alternative to reading file contents, we also support hexadecimal-encoded strings.\n\n"
                "Every byte is encoded as two digits without a `0x` prefix: 05 ab 13.\n"
                "The following showcases some examples of how this would look like:\n\n"
                "    - printrospector --hex \"f0 0d ba be\"\n"
                "    - printrospector --hex \"abcd1234f0f0\"\n\n"
                "Make sure to always quote your string in \"\" when using spaces as they will otherwise be "
                "interpreted as separate arguments.\n\n"
                "Note: When this option is followed by [--infile/-in], it will be ignored. Only one source "
                "of input is allowed at a time.",
                [](Options &opts, const char *value) {
                    opts.input_type = InputType::Hex;
                    opts.input_hex = value;
                }
            ),
            MakeProcessor(
                "infile", 'i', "specifies a path to a file that will be used as an input source",
                "As an alternative to specifying hexadecimal data with the [--hex] option, printrospector"
                "supports reading the contents of binary files by path.\n\n"
                "Relative and absolute paths are supported.\n\n"
                "Note: When this option is followed by [--hex], it will be ignored. Only one source of "
                "input is allowed at a time.",
                [](Options &opts, const char *value) {
                    opts.input_type = InputType::File;
                    opts.input_file = value;
                    return opts.input_file.has_filename();
                }
            ),
            MakeProcessor(
                "out", 'o', "specifies a path to the output file for (de)serialized contents",
                "When this option is missing, information will be printed to stdout on a"
                "best-effort basis without producing any persistent data.",
                [](Options &opts, const char *value) {
                    opts.output = value;
                    return opts.output.has_filename();
                }
            ),
            MakeProcessor(
                "type-list", 't', "specifies a wizwalker type list file",
                "The type list is a big JSON dump of type information crafted for ObjectProperty "
                "runtime reflection.\n\n"
                "Said file can be obtained using https://github.com/StarrFox/wizwalker. Refer "
                "to its GitHub page for installation and instructions. Once that is done, run\n\n"
                "    wizwalker dump json\n\n"
                "with an open instance of the game to obtain a game named similarly to "
                "r707528_Wizard_1_460.json.\n\n"
                "Note: When [--data-kind/-k] is not set to op, this option will be ignored.",
                [](Options &opts, const char *value) {
                    opts.type_list = value;
                    return opts.type_list.has_filename();
                }
            ),
            MakeProcessor(
                "serializer-type", 's', "the ObjectProperty serializer type to use",
                "This selects one of three different ObjectProperty binary serializer subclasses "
                "found throughout KingsIsle games:\n\n"
                "    - basic: what is known as SerializerBinary, this is the most commonly used instance\n"
                "    - core: what is known as SerializerCoreObjects, for in-game entities known as CoreObjects\n"
                "    - mannequin: what is known as SerializerMannequin, for mannequin objects\n\n"
                "Note: When [--data-kind/-k] is not set to op, this option will be ignored.",
                [](Options &opts, const char *value) {
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
                }
            ),
            MakeProcessor(
                "serializer-flags", 'f', "configuration bits for ObjectProperty serialization",
                "Serializer flags are represented as a bit mask on an integer. They actively "
                "influence the output of serialization/deserialization of data.\n\n"
                "The game supports the following bits:\n"
                "    - Bit 0: Stateful flags: The binary data store the configuration.\n"
                "    - Bit 1: Compact length prefixes: String and sequence lengths are stored size-efficient.\n"
                "    - Bit 2: Human readable enums: Instead of an integral value, string variants will be stored.\n"
                "    - Bit 3: Compressed: The data will be zlib-compressed for size efficiency.\n"
                "    - Bit 4: Require optional values: Optional values may not be missing in the state.\n\n"
                "In more practical terms, inputs ranging between 0 and 32 will be accepted. Greater values are "
                "truncated by default.\n\n"
                "Input can be either in decimal or in hexadecimal (using a 0x prefix).\n\n"
                "Note: When [--data-kind/-k] is not set to op, this option will be ignored.",
                [](Options &opts, const char *value) {
                    bool result = false;
                    opts.serializer_flags = IntParseHelper(value, result) & 0x1F;
                    return result;
                }
            ),
            MakeProcessor(
                "property-mask", 'm', "specifies a mask of property bit flags for ObjectProperty (de)serialization",
                "In the ObjectProperty system, every property has a set of bit flags assigned which are "
                "referred to as property flags.\n\n"
                "Given the mask, the serializer filters out all properties which are not an intersection of it.\n"
                "The default value is 0x18, you may specify any 32-bit value with this option instead.\n\n"
                "Input can be either in decimal or in hexadecimal (using a 0x prefix).\n\n"
                "Note: When [--data-kind/-k] is not set to op, this option will be ignored.",
                [](Options &opts, const char *value) {
                    bool success = false;
                    opts.property_mask = IntParseHelper(value, success);
                    return success;
                }
            ),
            MakeProcessor(
                "shallow", "forces shallow (de)serialization of ObjectProperty state",
                "ObjectProperty serialization supports a shallow and a deep mode.\n"
                "The shallow mode sequentially writes all values whereas deep mode tags each value with "
                "its hash and bit size to integrity-check.\n\n"
                "Most persistent state in files is serialized in deep mode; settings this option is "
                "usually not necessary.\n\n"
                "Note: When [--data-kind/-k] is not set to op, this option will be ignored.",
                [](Options &opts) { opts.shallow = true; }
            ),
            MakeProcessor(
                "manual-compression", 'c', "uncompress ObjectProperty state before processing",
                "Every so often it happens that ObjectProperty binary state gets compressed manually "
                "instead of utilizing the designated serializer configuration bit (see [--serializer-flags/-f]).\n\n"
                "The output then differs in a way that cannot be handled by specifying said flag which "
                "is the reason why this is a separate option.\n\n"
                "Note: When [--data-kind/-k] is not set to op, this option will be ignored.",
                [](Options &opts) { opts.manual_compression = true; }
            ),
            MakeProcessor(
                "quiet", 'q', "do all processing quietly",
                "By default, printrospector will log relevant details and progress to stdout/stderr.\n\n"
                "Users who find this behavior undesirable may specify this option for silent operation.",
                [](Options &opts) { opts.quiet = true; }
            ),
        };

        void PrintHelpHeader() {
            fmt::print("{} {}.{}.{} ({})\n", PTOR_NAME, PTOR_VERSION, PTOR_GIT_REV);
            fmt::print("Copyright (c) {}\n", PTOR_AUTHOR);
            fmt::print("{}\n\n", PTOR_DESCRIPTION);
        }

        void FormatOptionToBuf(std::string &out, const OptionProcessor &opt) {
            fmt::format_to(std::back_inserter(out), "--{}", opt.name);
            if (opt.short_name) {
                fmt::format_to(std::back_inserter(out), "/-{}", opt.short_name);
            }
            out += ':';
        }

        void HelpCommandImpl(Options &opts, const char *value) {
            P_UNUSED(opts);

            if (value == nullptr) {
                PrintUsage();
            } else {
                const auto value_len = std::strlen(value);
                for (const auto &opt : g_option_processors) {
                    /* Find the option that corresponds to what the user wants help for. */
                    if ((value_len == 1 && *value == opt.short_name) || std::strcmp(opt.name, value) == 0) {
                        PrintHelpHeader();

                        std::string buf;
                        FormatOptionToBuf(buf, opt);

                        fmt::print("{} {}\n\n{}\n", buf, opt.short_help, opt.help);
                    }
                }
            }

            /* At this point, we don't want to proceed parsing further arguments. */
            /* When we get --help, we can immediately opt out and call it a day.  */
            std::exit(1);
        }

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

    void PrintUsage() {
        PrintHelpHeader();

        fmt::print("Usage: {} [options...]\n\n", PTOR_NAME);

        fmt::print("Options:\n");

        std::string buf;
        for (const auto &opt : g_option_processors) {
            FormatOptionToBuf(buf, opt);
            fmt::print("    {:<30} {}\n", buf, opt.short_help);
            buf.clear();
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
                fmt::print(fg(fmt::color::yellow), "Warning: Failed to parse option \"{}\"!\n", arg);
                return {};
            }
        }

        /* We're valid when there's at least any input source. */
        if (options.input_type == InputType::Unknown) {
            return {};
        }

        return options;
    }

}
