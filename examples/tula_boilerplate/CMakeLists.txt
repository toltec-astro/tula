cmake_minimum_required(VERSION 3.25)

project(tula_boilerplate
    VERSION 3.1.0
    DESCRIPTION "Minimal Tula feature/provider workflow"
    LANGUAGES CXX
)

include(TulaProject)
tula_resolve_features()
tula_add_config_header(
    TARGET tula_boilerplate_config
    ALIAS tula_boilerplate::config
    PREFIX TULA_BOILERPLATE
    OUTPUT tula_boilerplate/config.h
)

add_library(tula_boilerplate_headers INTERFACE)
target_include_directories(
    tula_boilerplate_headers
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>"
)
target_link_libraries(tula_boilerplate_headers INTERFACE tula_boilerplate::config)
add_library(tula_boilerplate::headers ALIAS tula_boilerplate_headers)

add_executable(tula_boilerplate src/main.cpp)
target_link_libraries(tula_boilerplate PRIVATE tula_boilerplate::headers)
if(TARGET tula::logging)
    target_link_libraries(tula_boilerplate PRIVATE tula::logging)
endif()
target_compile_features(tula_boilerplate PRIVATE cxx_std_23)
set_target_properties(
    tula_boilerplate
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

install(TARGETS tula_boilerplate_headers EXPORT tula_boilerplate_targets)
install(DIRECTORY include/ DESTINATION include)
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/tula_boilerplate/config.h"
    DESTINATION include/tula_boilerplate
)
