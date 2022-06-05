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
#pragma once

#include <cstdio>
#include <memory>
#include <system_error>
#include <utility>

#include "ptor_defines.hpp"
#include "ptor_types.hpp"
#include "bin/cli_options.hpp"
#include "io/io_memory_mapped.hpp"
#include "wad/wad_types.hpp"

namespace ptor {

    class ContentProcessor final {
        P_DISALLOW_COPY_AND_ASSIGN(ContentProcessor);
        P_DISALLOW_MOVE(ContentProcessor);

    private:
        template <u32 Width>
        class ProgressBar {
            P_DISALLOW_COPY_AND_ASSIGN(ProgressBar);
            P_DISALLOW_MOVE(ProgressBar);

        private:
            static constexpr char Cell  = '=';
            static constexpr char Empty = ' ';

        private:
            const char *m_prefix;
            u32 m_cells;
            u32 m_current;
            u32 m_total;

        public:
            ProgressBar(const char *p, u32 total) : m_prefix{p}, m_cells{0}, m_current{0}, m_total{total} {
                this->Render();
            }

            ~ProgressBar() {
                fmt::print(" Done!\n");
            }

            void Update(size_t new_current) {
                m_current = new_current;

                if (const u32 cells = (m_current * Width) / m_total; cells != m_cells) {
                    m_cells = cells;
                    this->Render();
                }
            }

            void Render() {
                char bar[Width + 1];
                std::memset(bar, Cell, m_cells);
                std::memset(bar + m_cells, Empty, Width - m_cells);
                bar[Width] = 0;

                fmt::print("\r{} [{}] {}/{}", m_prefix, bar, m_current, m_total);
                std::fflush(stdout);
            }
        };

    private:
        cli::Options m_options;

    public:
        explicit ContentProcessor(cli::Options options);

        void Process(std::error_code &ec);

        void Save(std::error_code &ec);

    public:
        struct ProcessWadContext {
            u8 *raw_data;
            wad::Header header;
            std::unique_ptr<wad::File[]> files;
        };

    private:
        void ProcessWad(std::error_code &ec);
    };

}
