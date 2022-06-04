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
        assert.hpp
        assert.cpp
        ptor_defines.hpp
        ptor_types.hpp

        io/io_binary_buffer.hpp
        io/io_binary_buffer.cpp
        io/io_memory_mapped.hpp

        util/util_alignment.hpp
        util/util_byteorder.hpp
        util/util_encoding.hpp
        util/util_i_function.hpp
        util/util_literals.hpp
        util/util_scope_guard.hpp

        wad/wad_api.hpp
        wad/wad_api.cpp
        wad/wad_types.hpp

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

target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<PLATFORM_ID:Windows>:PTOR_OS_WINDOWS>
        $<$<PLATFORM_ID:Linux>:PTOR_OS_LINUX>
        $<$<PLATFORM_ID:Android>:PTOR_OS_ANDROID>
        $<$<PLATFORM_ID:Darwin>:PTOR_OS_APPLE>
        )

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources(${PROJECT_NAME} PRIVATE
            io/impl/io_memory_mapped.os.windows.hpp
            io/impl/io_memory_mapped.os.windows.cpp
            )
else()
    target_sources(${PROJECT_NAME} PRIVATE
            io/impl/io_memory_mapped.unix.hpp
            io/impl/io_memory_mapped.unix.cpp
            )
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt libdeflate::deflate)

# Enable debug assertions when not building in some release mode.
target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:P_ENABLE_DEBUG_ASSERTIONS>
        )

target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        )

# Install the printrospector CLI to system, when requested.
if(PTOR_OPT_INSTALL)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
