cmake_minimum_required(VERSION 3.25)

project(tula_fitting_consumer LANGUAGES CXX)

find_package(tula 3.1 CONFIG REQUIRED COMPONENTS fitting)

add_executable(tula_fitting_consumer main.cpp)
target_compile_features(tula_fitting_consumer PRIVATE cxx_std_23)
target_link_libraries(tula_fitting_consumer PRIVATE tula::fitting)

enable_testing()
add_test(NAME tula_fitting_consumer COMMAND tula_fitting_consumer)
