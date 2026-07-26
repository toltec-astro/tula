cmake_minimum_required(VERSION 3.25)
project(tula_package_test LANGUAGES CXX)

find_package(tula CONFIG REQUIRED)

add_executable(tula_package_test src/main.cpp)
target_link_libraries(tula_package_test PRIVATE tula::headers)
target_compile_features(tula_package_test PRIVATE cxx_std_23)
