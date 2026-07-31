cmake_minimum_required(VERSION 3.25)

project(tula_perflibs_consumer LANGUAGES CXX)

set(
    EXPECT_OPENMP
    ""
    CACHE STRING
    "Expected Tula OpenMP capability (0 or 1)"
)
if(NOT EXPECT_OPENMP MATCHES "^[01]$")
    message(FATAL_ERROR "EXPECT_OPENMP must be 0 or 1")
endif()

find_package(tula 3.1 CONFIG REQUIRED COMPONENTS perflibs)

add_executable(tula_perflibs_consumer main.cpp)
target_compile_features(tula_perflibs_consumer PRIVATE cxx_std_23)
target_compile_definitions(
    tula_perflibs_consumer
    PRIVATE EXPECT_OPENMP=${EXPECT_OPENMP}
)
target_link_libraries(
    tula_perflibs_consumer
    PRIVATE tula::perflibs
)

enable_testing()
add_test(NAME tula_perflibs_consumer COMMAND tula_perflibs_consumer)
