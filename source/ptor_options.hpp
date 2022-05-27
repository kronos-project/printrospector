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

#include "ptor_defines.hpp"
#include "ptor_types.hpp"

namespace ptor {

    enum class SerializeOpt {
        Serialize,
        Deserialize,
    };

    enum class InputType {
        ObjectProperty_Hex,
        ObjectProperty_File,

        Unknown,
    };

    enum class SerializerType {
        Basic,
        CoreObject,
        Mannequin,
    };

    struct Options {
        /* Input/output sources for data to serialize/deserialize. */
        SerializeOpt serialize_opt = SerializeOpt::Deserialize;
        InputType input_type = InputType::Unknown;
        const char *input = nullptr;
        const char *output = nullptr;

        /* Path to the wizwalker type list. */
        const char *type_list = nullptr;

        /* Binary serializer configuration. */
        SerializerType serializer_type = SerializerType::Basic;
        u32 serializer_flags = 0;
        u32 property_mask = 0x18;
        bool shallow = false;
        bool manual_compression = false;

        /* Don't log during processing. */
        bool quiet = false;

        /* Skip unknown types during serialization/deserialization. */
        bool skip_unknown = false;
    };

    std::optional<Options> ParseOptionsFromArgs(int argc, char **argv);

}