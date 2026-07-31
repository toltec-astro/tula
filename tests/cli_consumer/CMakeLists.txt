cmake_minimum_required(VERSION 3.25)

project(tula_cli_consumer LANGUAGES CXX)

find_package(tula 3.1 CONFIG REQUIRED COMPONENTS cli)

add_executable(tula_cli_consumer main.cpp)
target_compile_features(tula_cli_consumer PRIVATE cxx_std_23)
target_link_libraries(tula_cli_consumer PRIVATE tula::cli)

enable_testing()
add_test(NAME tula_cli_consumer COMMAND tula_cli_consumer)
