cmake_minimum_required(VERSION 4.1)

project(cli_builder
    DESCRIPTION "tula v3 example: CLI builder with config and grppi"
    LANGUAGES CXX C
)

add_executable(cli_builder)
target_sources(cli_builder PRIVATE main.cpp)
target_link_libraries(cli_builder
    PRIVATE
        tula::headers
        tula::logging
        tula::Clipp
        tula::Grppi
        tula::Enum
        tula::Yaml
)
target_compile_features(cli_builder PRIVATE cxx_std_23)
