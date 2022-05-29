cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

######################
## Project settings ##
######################

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(printrospector
        DESCRIPTION "A tool for working with KingsIsle data formats"
        VERSION "0.1.0"
        HOMEPAGE_URL "https://github.com/kronos-project/printrospector"
        )

include(GitRevision)
message(STATUS "Compiling ${PROJECT_NAME} ${PROJECT_VERSION}, git revision ${PTOR_GIT_REV}")

#########################
## CMake configuration ##
#########################

# Nicely support folders inside IDEs.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Disallow in-source builds of the project.
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR
            "In-source builds are not allowed.\n"
            "Please create a new directory and run cmake from there."
    )
endif()

# Set the CMake build type, if not provided by the user.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type." FORCE)
endif()

# Define configurable build options.
option(PTOR_OPT_INSTALL "Install the printrospector CLI target" OFF) # TODO

# Enforce the C++ standard when this is the top-level project.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

####################
## Subdirectories ##
####################

add_subdirectory(third-party)
add_subdirectory(source)
