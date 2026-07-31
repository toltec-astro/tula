cmake_minimum_required(VERSION 3.25)

project(tula_ecsv_reader LANGUAGES CXX)

find_package(
    tula 3.1 CONFIG REQUIRED
    COMPONENTS ecsv
)

add_executable(tula_ecsv_reader main.cpp)
target_compile_features(tula_ecsv_reader PRIVATE cxx_std_23)
target_link_libraries(
    tula_ecsv_reader
    PRIVATE tula::ecsv
)
