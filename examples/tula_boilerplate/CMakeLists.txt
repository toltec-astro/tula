cmake_minimum_required(VERSION 4.1)

project(tula_boilerplate
    DESCRIPTION "Minimum working example for tula v3"
    LANGUAGES CXX
)

# tula::tula is provided by the Conan toolchain (TulaConan).
# It bundles tula headers + all enabled dependency targets.

add_executable(tula_boilerplate)
target_sources(tula_boilerplate PRIVATE src/main.cpp)
target_link_libraries(tula_boilerplate PRIVATE tula::tula)
target_compile_features(tula_boilerplate PRIVATE cxx_std_23)
