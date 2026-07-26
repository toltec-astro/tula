cmake_minimum_required(VERSION 3.25)

project(tula
    VERSION 3.1.0
    DESCRIPTION "The C++ utility library for TolTEC."
    HOMEPAGE_URL "https://toltec-astro.github.io/tula"
    LANGUAGES CXX
)

include(TulaProject)
tula_resolve_features()
tula_add_config_header(
    TARGET tula_config
    ALIAS tula::config
    PREFIX TULA
    OUTPUT tula/config.h
)

add_library(tula_headers INTERFACE)
target_include_directories(
    tula_headers
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>"
)
target_link_libraries(tula_headers INTERFACE tula::config)
if(TARGET tula::logging)
    target_link_libraries(tula_headers INTERFACE tula::logging)
endif()
if(TARGET tula::yaml_cpp)
    target_link_libraries(tula_headers INTERFACE tula::yaml_cpp)
endif()
if(TARGET tula::csv_parser)
    target_link_libraries(tula_headers INTERFACE tula::csv_parser)
endif()
if(TARGET tula::netcdf_cxx4)
    target_link_libraries(tula_headers INTERFACE tula::netcdf_cxx4)
endif()
if(TARGET tula::bitmask)
    target_link_libraries(tula_headers INTERFACE tula::bitmask)
endif()
if(TARGET tula::meta_enum)
    target_link_libraries(tula_headers INTERFACE tula::meta_enum)
endif()
if(TARGET tula::clipp)
    target_link_libraries(tula_headers INTERFACE tula::clipp)
endif()
if(TARGET tula::perflibs)
    target_link_libraries(tula_headers INTERFACE tula::perflibs)
endif()
if(TARGET tula::eigen)
    target_link_libraries(tula_headers INTERFACE tula::eigen)
endif()
if(TARGET tula::grppi)
    target_link_libraries(tula_headers INTERFACE tula::grppi)
endif()
add_library(tula::headers ALIAS tula_headers)

install(TARGETS tula_headers EXPORT tula_targets)
install(DIRECTORY include/ DESTINATION include)
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/tula/config.h"
    DESTINATION include/tula
)
tula_install_bundled_headers()

option(TULA_BUILD_TESTS "Compile and run the currently supported Tula header surface" ON)
if(TULA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
