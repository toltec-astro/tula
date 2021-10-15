project (tula_test_cli_builder LANGUAGES CXX C)

add_executable(test_cli_builder EXCLUDE_FROM_ALL)
set_target_properties(test_cli_builder
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
target_sources(test_cli_builder
    PRIVATE
        main.cpp
        )
target_link_libraries(test_cli_builder
    PRIVATE
        tula::headers
        tula::logging
        tula::Clipp
        tula::Grppi
        tula::Enum
        )
add_dependencies(test_progs test_cli_builder)
