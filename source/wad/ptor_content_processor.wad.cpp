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

#include "io/io_binary_buffer.hpp"
#include "io/io_memory_mapped.hpp"
#include "util/util_scope_guard.hpp"
#include "util/util_zlib_inflater.hpp"
#include "wad/wad_api.hpp"

namespace ptor {

    namespace {

        void WriteFile(const fs::path &outdir, const wad::File &file, const u8 *data, std::error_code &ec) {
            const fs::path &outfile = outdir / file.path;

            /* Attempt to create the directory for the output file. */
            if (fs::create_directories(outfile.parent_path(), ec); ec) {
                return;
            }

            /* Write the file contents. */
            FILE *fp = std::fopen(outfile.string().c_str(), "wb");
            if (fp != nullptr) {
                if (std::fwrite(data, sizeof(u8), file.uncompressed_size, fp) != file.uncompressed_size) {
                    ec = std::make_error_code(std::errc::io_error);
                }
                std::fclose(fp);
            } else {
                ec = std::make_error_code(std::errc::invalid_argument);
            }
        }

        void ExtractArchive(ContentProcessor::ProcessWadContext &ctx, u8 *data, size_t len, fs::path &out, std::error_code &ec) {
            io::BinaryBuffer buffer{data, len};
            auto inflater = util::Inflater::Allocate(ec);
            if (ec) {
                return;
            }

            /* Initialize the context. */
            ctx.raw_data = data;
            ctx.header   = wad::ReadHeader(buffer);
            ctx.files    = std::make_unique<wad::File[]>(static_cast<size_t>(ctx.header.file_count));

            /* Deserialize all the file structures. */
            for (u32 i = 0; i < ctx.header.file_count; ++i) {
                ctx.files[i] = wad::ReadFile(buffer);
            }

            /* Create the output directory for the extracted files, if not exists. */
            if (fs::create_directories(out, ec); ec) {
                return;
            }

            /* Extract WAD archive file by file. */
            ContentProcessor::ProgressBar<60> progress_bar{"Extracting KIWAD archive...", ctx.header.file_count};
            for (u32 i = 1; i <= ctx.header.file_count; ++i) {
                const auto &file    = ctx.files[i - 1];
                const auto contents = wad::GetFileContents(file, ctx.raw_data);

                /* Decompress the file contents, if necessary. */
                const u8 *decompressed = contents.data();
                if (file.compressed) {
                    if (inflater.Decompress(decompressed, contents.size_bytes(), file.uncompressed_size, ec); ec) {
                        return;
                    }
                    decompressed = inflater.GetCurrentBufferPtr();
                }

                /* Write the decompressed file to disk. */
                WriteFile(out, file, decompressed, ec);

                /* Advance the progress bar every 10 files to lower contention. */
                if (i % 10 == 0) {
                    progress_bar.Update(i);
                }
            }
        }

    }

    void ContentProcessor::ProcessWad(std::error_code &ec) {
        /* Create the context for processing. */
        ProcessWadContext ctx{};

        if (m_options.input_type == cli::InputType::File) {
            /* Attempt to open the supplied input source. */
            FILE *input = std::fopen(m_options.input_file.string().c_str(), "rb");
            if (input == nullptr) {
                ec = std::make_error_code(std::errc::no_such_file_or_directory);
                return;
            }
            P_ON_SCOPE_EXIT { std::fclose(input); };

            /* Memory-map the file contents. */
            auto mapped = io::ReadOnlyMapped::Map(input, ec);
            if (ec) {
                return;
            }

            /* Do the extraction work. */
            ExtractArchive(ctx, mapped.GetPtr(), mapped.GetLength(), m_options.output, ec);
        } else {
            P_DEBUG_ASSERT(m_options.input_type == cli::InputType::Hex);

            P_TODO();
        }
    }

}
