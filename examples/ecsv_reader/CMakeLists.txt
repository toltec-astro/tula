cmake_minimum_required(VERSION 4.1)

project(ecsv_reader
    DESCRIPTION "tula v3 example: ECSV table reader"
    LANGUAGES CXX C
)

add_executable(ecsv_reader)
target_sources(ecsv_reader PRIVATE main.cpp)
target_link_libraries(ecsv_reader
    PRIVATE
        tula::headers
        tula::logging
        tula::Enum
        tula::Clipp
        tula::Yaml
        tula::Eigen3
        tula::Csv
)
target_compile_features(ecsv_reader PRIVATE cxx_std_23)
set_target_properties(ecsv_reader PROPERTIES
    DISABLE_PRECOMPILE_HEADERS ON
)
