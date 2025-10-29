cmake_minimum_required(VERSION 4.1)

project(tula
    DESCRIPTION "The C++ utility library for TolTEC."
    HOMEPAGE_URL "https://toltec-astro.github.io/tula"
    LANGUAGES CXX
)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/tula_cmake/cmake" ${CMAKE_MODULE_PATH})
include(tula_cmake)

# Header-only interface library
add_library(tula_headers INTERFACE)
target_include_directories(tula_headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(generate_config_header)
target_link_libraries(tula_headers INTERFACE tula::config_header)

file(GLOB_RECURSE TULA_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/tula/*.h")
target_precompile_headers(tula_headers INTERFACE ${TULA_HEADER_FILES})

add_library(tula::headers ALIAS tula_headers)

# Standalone build configuration
if(NOT DEFINED TULA_STANDALONE)
    set(TULA_STANDALONE $<STREQUAL:${CMAKE_CURRENT_SOURCE_DIR},${CMAKE_SOURCE_DIR}>)
endif()

if(TULA_STANDALONE)
    # Phase 1: Register dependencies (each package calls tula_deps_register)
    # include(bitmask)
    include(Eigen3)  # Testing new naming convention architecture
    # include(perflibs)
    # include(logging)  # Registers fmt and spdlog
    include(Yaml)  # Testing v3 migration
    include(Spectra)  # Testing v3 migration
    include(Ceres)  # Testing v3 migration with Eigen3 shim
    # include(NetCDFCXX4)
    # include(MXX)
    # include(CCfits)
    
    # Phase 2: Create targets for all registered dependencies
    # This will invoke TRY_* functions for each package with automatic fallback
    if(COMMAND tula_deps_create_targets)
        tula_deps_create_targets()
    endif()
    
    # Phase 3: No longer needed - v3 architecture calls CREATE_WRAPPER automatically
    
    set(component_libs
        tula::perflibs
        # tula::logging
        tula::Yaml
        # tula::NetCDFCXX4
        tula::Eigen3
        # tula::Ceres  # TODO: Fix export issue
        # tula::MXX
        # tula::CCfits
    )

    # Aggregate target for testing all modules
    add_library(tula_all INTERFACE)
    target_link_libraries(tula_all INTERFACE tula::headers ${component_libs})
    add_library(tula::tula ALIAS tula_all)

    # Build options
    option(TULA_BUILD_TESTS "Build tests" ON)
    if(TULA_BUILD_TESTS)
        add_subdirectory(tests)
    endif()

    option(TULA_BUILD_EXAMPLES "Build example programs" OFF)
    if(TULA_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()

    option(TULA_BUILD_DOC "Build documentation" OFF)
    if(TULA_BUILD_DOC)
        include(docxgen_target)
    endif()
endif()

# Generate configuration headers
generate_gitversion_header(${PROJECT_NAME})
generate_config_header(${PROJECT_NAME})
