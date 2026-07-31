cmake_minimum_required(VERSION 3.25)
project(tula_installed_consumer LANGUAGES CXX)

find_package(tula CONFIG REQUIRED COMPONENTS core netcdf)

add_executable(tula_installed_consumer main.cpp)
target_link_libraries(
    tula_installed_consumer
    PRIVATE
        tula::core
        tula::netcdf
)
target_compile_features(tula_installed_consumer PRIVATE cxx_std_23)

enable_testing()
add_test(NAME tula_installed_consumer COMMAND tula_installed_consumer)
