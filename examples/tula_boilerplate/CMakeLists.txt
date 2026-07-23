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

add_executable(tula_boilerplate src/main.cpp)
target_link_libraries(tula_boilerplate PRIVATE tula_boilerplate::config)
if(TARGET tula::logging)
    target_link_libraries(tula_boilerplate PRIVATE tula::logging)
endif()
target_compile_features(tula_boilerplate PRIVATE cxx_std_23)
set_target_properties(
    tula_boilerplate
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
