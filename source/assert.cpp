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

#include "assert.hpp"

#include <exception>

#include "fmt/color.h"

namespace ptor::impl {

    void AssertFailImpl(const SourceLocation &loc, const char *expr) {
        fmt::print(stderr, fg(fmt::color::red), "Assertion Failure!\n");
        fmt::print(stderr, fg(fmt::color::red), "    Expression: {}\n", expr);
        fmt::print(stderr, fg(fmt::color::red), "    Function:   {}\n", loc.func);
        fmt::print(stderr, fg(fmt::color::red), "    Location:   {}:{}\n", loc.file, loc.line);
        std::terminate();
    }

    void ReifiedAssertFailImpl(const SourceLocation &loc, const char *expr, std::string_view format, fmt::format_args args) {
        fmt::print(stderr,  fg(fmt::color::red), "Assertion Failure: ");
        fmt::vprint(stderr, fg(fmt::color::red), format, args);
        fmt::print(stderr,  fg(fmt::color::red), "!\n");
        fmt::print(stderr,  fg(fmt::color::red), "    Expression: {}\n", expr);
        fmt::print(stderr,  fg(fmt::color::red), "    Function:   {}\n", loc.func);
        fmt::print(stderr,  fg(fmt::color::red), "    Location:   {}:{}\n", loc.file, loc.line);
        std::terminate();
    }

}
