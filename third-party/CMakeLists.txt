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
