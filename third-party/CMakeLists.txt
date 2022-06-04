macro(check_submodules)
    # Read and extract all modules from .gitmodules.
    file(READ "${CMAKE_SOURCE_DIR}/.gitmodules" __GITMODULES)
    string(REGEX MATCHALL "path *= *[^\t\r\n]*" __GITMODULES ${__GITMODULES})

    # Verify that each module has a .git directory in its path,
    # i.e. ensure that all submodules are initialized and available.
    foreach(module ${__GITMODULES})
        string(REGEX REPLACE "path *= *" "" module ${module})
        if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${module}/.git")
            message(FATAL_ERROR "${module} not found. Run git submodule update --init --recursive")
        endif()
    endforeach()
endmacro()

# Make sure all submodules were cloned along with the repository.
if(EXISTS ${CMAKE_SOURCE_DIR}/.gitmodules)
    check_submodules()
endif()

# fmt
add_subdirectory(fmt)

# libdeflate
add_library(libdeflate
        libdeflate/common/common_defs.h
        libdeflate/common/compiler_gcc.h
        libdeflate/common/compiler_msc.h

        libdeflate/lib/arm/adler32_impl.h
        libdeflate/lib/arm/cpu_features.c
        libdeflate/lib/arm/cpu_features.h
        libdeflate/lib/arm/crc32_impl.h
        libdeflate/lib/arm/matchfinder_impl.h

        libdeflate/lib/x86/adler32_impl.h
        libdeflate/lib/x86/cpu_features.c
        libdeflate/lib/x86/cpu_features.h
        libdeflate/lib/x86/crc32_impl.h
        libdeflate/lib/x86/crc32_pclmul_template.h
        libdeflate/lib/x86/decompress_impl.h
        libdeflate/lib/x86/matchfinder_impl.h

        libdeflate/lib/adler32.c
        libdeflate/lib/adler32_vec_template.h
        libdeflate/lib/bt_matchfinder.h
        libdeflate/lib/cpu_features_common.h
        libdeflate/lib/crc32.c
        libdeflate/lib/crc32_table.h
        libdeflate/lib/crc32_vec_template.h
        libdeflate/lib/deflate_compress.c
        libdeflate/lib/deflate_compress.h
        libdeflate/lib/deflate_constants.h
        libdeflate/lib/deflate_decompress.c
        libdeflate/lib/gzip_compress.c
        libdeflate/lib/gzip_constants.h
        libdeflate/lib/gzip_decompress.c
        libdeflate/lib/hc_matchfinder.h
        libdeflate/lib/ht_matchfinder.h
        libdeflate/lib/matchfinder_common.h
        libdeflate/lib/unaligned.h
        libdeflate/lib/utils.c
        libdeflate/lib/zlib_compress.c
        libdeflate/lib/zlib_constants.h
        libdeflate/lib/zlib_decompress.c

        libdeflate/libdeflate.h
        )
add_library(libdeflate::deflate ALIAS libdeflate)

target_include_directories(libdeflate
        PUBLIC libdeflate
        PRIVATE libdeflate/lib
        )
