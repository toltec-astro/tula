cmake_minimum_required(VERSION 3.25)

project(tula_downstream
    VERSION 3.1.0
    DESCRIPTION "Minimal downstream consumer of a Tula-built package"
    LANGUAGES CXX
)

find_package(tula_boilerplate CONFIG REQUIRED)

add_executable(tula_downstream src/main.cpp)
target_link_libraries(
    tula_downstream
    PRIVATE tula_boilerplate::headers
)
target_compile_features(tula_downstream PRIVATE cxx_std_23)
set_target_properties(
    tula_downstream
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
