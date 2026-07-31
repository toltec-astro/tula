cmake_minimum_required(VERSION 3.25)

project(tula_enum_consumer LANGUAGES CXX)

find_package(tula 3.1 CONFIG REQUIRED COMPONENTS enum)

add_executable(tula_enum_consumer main.cpp)
target_compile_features(tula_enum_consumer PRIVATE cxx_std_23)
target_link_libraries(tula_enum_consumer PRIVATE tula::enum)

enable_testing()
add_test(NAME tula_enum_consumer COMMAND tula_enum_consumer)
