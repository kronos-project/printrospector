/*
 * Copyright (c) 2022 Valentin B.
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

#include "bin/ptor_content_processor.hpp"

namespace ptor {

    ContentProcessor::ContentProcessor(cli::Options options) : m_options{std::move(options)} {}

    void ContentProcessor::Process(std::error_code &ec) {
        P_DEBUG_ASSERT(m_options.encode_opt == cli::EncodeOpt::Decode);

        /* Reset the error code back into a successful state. */
        ec.clear();

        /* Decode the format we got. */
        switch (m_options.data_kind) {
            case cli::DataKind::ObjectProperty: P_TODO(); break;
            case cli::DataKind::Wad:            return this->ProcessWad(ec);

            default: P_UNREACHABLE();
        }
    }

    void ContentProcessor::Save(std::error_code &ec) {
        P_DEBUG_ASSERT(m_options.encode_opt == cli::EncodeOpt::Encode);

        /* Reset the error code back into a successful state. */
        ec.clear();

        P_TODO();
    }

}
