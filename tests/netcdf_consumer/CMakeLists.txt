cmake_minimum_required(VERSION 3.25)

project(tula_netcdf_consumer LANGUAGES CXX)

find_package(tula 3.1 CONFIG REQUIRED COMPONENTS netcdf)

add_executable(tula_netcdf_consumer main.cpp)
target_compile_features(tula_netcdf_consumer PRIVATE cxx_std_23)
target_link_libraries(tula_netcdf_consumer PRIVATE tula::netcdf)

enable_testing()
add_test(NAME tula_netcdf_consumer COMMAND tula_netcdf_consumer)
