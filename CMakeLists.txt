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
    INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
target_link_libraries(tula_headers INTERFACE tula::config)
if(TARGET tula::formatting)
    target_link_libraries(tula_headers INTERFACE tula::formatting)
endif()
if(TARGET tula::logging)
    target_link_libraries(tula_headers INTERFACE tula::logging)
endif()
add_library(tula::headers ALIAS tula_headers)

option(TULA_BUILD_TESTS "Compile and run the currently supported Tula header surface" ON)
if(TULA_BUILD_TESTS)
    enable_testing()
    add_executable(tula_header_smoke tests/header_smoke.cpp)
    target_link_libraries(tula_header_smoke PRIVATE tula::headers)
    target_compile_features(tula_header_smoke PRIVATE cxx_std_23)
    add_test(NAME tula::header_smoke COMMAND tula_header_smoke)
endif()
