############################
## Compiler configuration ##
############################

if(MSVC)
    # Silence "deprecation" warnings.
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)

    add_compile_options(
        /W4
        /MP
        /Zi
        /Zm200
        /Zo
        /permissive-
        /EHsc
        /GR-
        /utf-8
        /volatile:iso
        /Zc:inline
        /Zc:externConstexpr
        /GT
    )

    set(CMAKE_EXE_LINKER_FLAGS_DEBUG   "/DEBUG /MANIFEST:NO" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/DEBUG /MANIFEST:NO /INCREMENTAL:NO /OPT:REF,ICF" CACHE STRING "" FORCE)
else()
    add_compile_options(
        -Wall
        -Werror=array-bounds
        -Werror=implicit-fallthrough
        -Werror=missing-declarations
        -Werror=missing-field-initializers
        -Werror=reorder
        -Werror=sign-compare
        -Werror=switch
        -Werror=uninitialized
        -Werror=unused-function
        -Werror=unused-result
        -Werror=unused-variable
        -Wextra
        -Wmissing-declarations
        -Wno-attributes
        -Wno-invalid-offsetof
        -Wno-unused-parameter

        -fno-exceptions
        -fno-unwind-tables
        -fno-asynchronous-unwind-tables
        -fno-rtti
    )

    # Set file offset to 64 bits.
    # Modern UNIXes mostly do that out of the box already, except glibc.
    # glibc however allows this to be configured explicitly.
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR MINGW)
        add_definitions(-D_FILE_OFFSET_BITS=64)
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR MINGW)
        # GNU ar: Create thin archive files.
        # Requires binutils-2.19 or later.
        set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> qcTP <TARGET> <LINK_FLAGS> <OBJECTS>")
        set(CMAKE_C_ARCHIVE_APPEND   "<CMAKE_AR> qTP  <TARGET> <LINK_FLAGS> <OBJECTS>")
        set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcTP <TARGET> <LINK_FLAGS> <OBJECTS>")
        set(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> qTP  <TARGET> <LINK_FLAGS> <OBJECTS>")
    endif()
endif()

###########################
## printrospector target ##
###########################

add_executable(${PROJECT_NAME}
        ptor_defines.hpp
        ptor_types.hpp

        util/util_alignment.hpp
        util/util_byteorder.hpp
        util/util_encoding.hpp
        util/util_i_function.hpp

        bin/cli_option_processor.hpp
        bin/cli_options.hpp
        bin/cli_options.cpp
        bin/ptor_main.cpp
        )

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ptor_version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/ptor_version.hpp
    @ONLY
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # We can't use pretty colors for logging on Windows by default.
    # Thus, we need this definition in order to fix it.
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DPTOR_OS_WINDOWS=1)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        )

# Install the printrospector CLI to system, when requested.
if(PTOR_OPT_INSTALL)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
