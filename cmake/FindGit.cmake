include(FindPackageHandleStandardArgs)

find_program(GIT_BINARY
        NAMES git
        DOC "git cli"
        )

find_package_handle_standard_args(Git
        REQUIRED_VARS GIT_BINARY
        )

mark_as_advanced(GIT_BINARY)
