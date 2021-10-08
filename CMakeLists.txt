cmake_minimum_required(VERSION 3.20)
project(tula
    DESCRIPTION
        "The C++ utility library for TolTEC"
    HOMEPAGE_URL
        "https://toltec-astro.github.io/tula"
    )

# Load CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/tula_cmake/cmake" ${CMAKE_MODULE_PATH})
include(common_settings)

add_library(tula_headers INTERFACE)
target_include_directories(tula_headers
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Do not add any link libs since we'll be pulling in dependencies
# as they are needed in the user code
add_library(tula::headers ALIAS tula_headers)

# Check if this project is embedded as sub project.
if(NOT DEFINED TULA_STANDALONE)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(TULA_STANDALONE ON)
    else()
        set(TULA_STANDALONE OFF)
    endif()
endif()

if (TULA_STANDALONE)

    # Create a sum-target tula_all incluing all componnets
    set(component_libs "")

    include(perflibs)
    set(component_libs ${component_libs} tula::perflibs)

    include(logging)
    set(component_libs ${component_libs} tula::logging)

    include(Yaml)
    set(component_libs ${component_libs} tula::Yaml)

    include(Clipp)
    set(component_libs ${component_libs} tula::Clipp)

    include(NetCDFCXX4)
    set(component_libs ${component_libs} tula::NetCDFCXX4)

    include(Eigen3)
    set(component_libs ${component_libs} tula::Eigen3)

    include(Ceres)
    set(component_libs ${component_libs} tula::Ceres)

    include(Grppi)
    set(component_libs ${component_libs} tula::Grppi)

    include(Csv)
    set(component_libs ${component_libs} tula::Csv)

    include(config_header)
    generate_gitversion_header(${CMAKE_PROJECT_NAME})
    generate_config_header(${CMAKE_PROJECT_NAME})
    set(component_libs ${component_libs} tula::config_header)

    add_library(tula_all INTERFACE)
    target_link_libraries(tula_all
        INTERFACE
            tula::headers
            ${component_libs}
        )
    add_library(tula::tula ALIAS tula_all)
    option(TULA_BUILD_TESTS "Build tests" ON)
    if (TULA_BUILD_TESTS)
        add_subdirectory(tests)
    endif()

    option(TULA_BUILD_EXAMPLES "Build example programs" OFF)
    if (TULA_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()

    option(TULA_BUILD_DOC "Build docs" OFF)
    if (TULA_BUILD_DOC)
        include(docxgen_target)
    endif()
endif()
