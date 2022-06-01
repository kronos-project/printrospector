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

#ifdef PTOR_OS_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

#include "bin/cli_options.hpp"

void EnableConsoleColors();

int main(int argc, char **argv) {
    /* Enable colored console output for logging on Windows. */
    EnableConsoleColors();

    /* Parse command line options. */
    const auto options = ptor::cli::ParseOptionsFromArgs(argc, argv);
    if (!options) {
        ptor::cli::PrintUsage();
        return 1;
    }

    return 0;
}

void EnableConsoleColors() {
#ifdef PTOR_OS_WINDOWS
    DWORD mode = 0;
    HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE stderr_handle = GetStdHandle(STD_ERROR_HANDLE);

    /* Enable colored output for stdout. */
    GetConsoleMode(stdout_handle, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(stdout_handle, mode);

    /* Enable colored output for stderr. */
    GetConsoleMode(stderr_handle, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(stderr_handle, mode);
#endif
}
