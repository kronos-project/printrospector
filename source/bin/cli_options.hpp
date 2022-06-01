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

#include <optional>

#include "ptor_types.hpp"

namespace ptor::cli {

    enum class EncodeOpt {
        Encode,
        Decode,
    };

    enum class InputType {
        Hex,
        File,

        Unknown,
    };

    enum class DataKind {
        ObjectProperty,
    };

    enum class SerializerType {
        Basic,
        CoreObject,
        Mannequin,
    };

    struct Options {
        /* Input/output sources for data to serialize/deserialize. */
        EncodeOpt encode_opt = EncodeOpt::Decode;
        InputType input_type = InputType::Unknown;
        DataKind data_kind = DataKind::ObjectProperty;
        const char *input_hex = nullptr;
        fs::path input_file{};
        fs::path output{};

        /* Path to the wizwalker type list. */
        fs::path type_list{};

        /* Binary serializer configuration. */
        SerializerType serializer_type = SerializerType::Basic;
        u32 serializer_flags = 0;
        u32 property_mask = 0x18;
        bool shallow = false;
        bool manual_compression = false;

        /* Don't log during processing. */
        bool quiet = false;
    };

    void PrintUsage();

    std::optional<Options> ParseOptionsFromArgs(int argc, char **argv);

}
