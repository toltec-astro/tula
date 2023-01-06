project (tula_test_ecsv_reader LANGUAGES CXX C)

add_executable(test_ecsv_reader EXCLUDE_FROM_ALL)
set_target_properties(test_ecsv_reader
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
target_sources(test_ecsv_reader
    PRIVATE
        main.cpp
        )
target_link_libraries(test_ecsv_reader
    PRIVATE
        tula::headers
        tula::logging
        tula::Clipp
        tula::Yaml
        tula::Csv
        )
add_dependencies(test_progs test_ecsv_reader)
