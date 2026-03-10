cmake_minimum_required(VERSION 4.1)

project(tula
    DESCRIPTION "The C++ utility library for TolTEC."
    HOMEPAGE_URL "https://toltec-astro.github.io/tula"
    LANGUAGES CXX
)

# ----------------------------------------------------------------------------
# tula v3: Conan-centric build system
#
# The Conan toolchain (conan_toolchain.cmake) handles everything:
#   - Includes tula_sensible.cmake (compiler defaults, RPATH, CPM cache)
#   - Includes tula_deps.cmake (tula_deps_add API + adds tula_cmake/utils to MODULE_PATH)
#   - Loads all enabled packages into TULA_DEPS cache variable
#
# CMakeLists.txt just defines the library targets and build options.
# ----------------------------------------------------------------------------

# tula_cmake/utils is on CMAKE_MODULE_PATH via tula_deps.cmake in the toolchain.
# generate_config_header defines: generate_gitversion_header() and generate_config_header()
# which create tula::config_header and write gitversion.h + config.h into build/.
include(generate_config_header)
generate_gitversion_header(${PROJECT_NAME})
generate_config_header(${PROJECT_NAME})

# Header-only interface library
add_library(tula_headers INTERFACE)
target_include_directories(tula_headers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(tula_headers INTERFACE tula::config_header)
add_library(tula::headers ALIAS tula_headers)

# Aggregate library: tula::tula = headers + all enabled dependencies.
# TULA_DEPS is populated automatically by the Conan toolchain for each
# enabled package (e.g. -o "&:Eigen3=conan" adds tula::Eigen3 to TULA_DEPS).
add_library(tula_all INTERFACE)
target_link_libraries(tula_all INTERFACE tula::headers ${TULA_DEPS})
add_library(tula::tula ALIAS tula_all)

option(TULA_BUILD_TESTS "Build tests" ON)
if(TULA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

option(TULA_BUILD_EXAMPLES "Build examples" OFF)
if(TULA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

option(TULA_BUILD_DOC "Build documentation" OFF)
if(TULA_BUILD_DOC)
    include(docxgen_target)
endif()
