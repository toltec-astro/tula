project (tula_test_mpi LANGUAGES CXX C)

add_executable(test_mpi EXCLUDE_FROM_ALL)
set_target_properties(test_mpi
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
target_sources(test_mpi
    PRIVATE
        main.cpp
        )
target_link_libraries(test_mpi
    PRIVATE
        tula::headers
        tula::logging
        tula::MXX
        tula::Grppi
        tula::Eigen3
        )
add_dependencies(test_progs test_mpi)
