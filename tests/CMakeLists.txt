project (tula_test LANGUAGES CXX C)

include(testing)

add_executable(tula_test)
set_target_properties(tula_test
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
target_sources(tula_test
    PRIVATE
        main.cpp
        eigen.cpp
        grppiex.cpp
        formatter/test_enum.cpp
        formatter/test_ptr.cpp
        formatter/test_byte.cpp
        formatter/test_duration.cpp
        formatter/test_container.cpp
        formatter/test_matrix.cpp
    )
target_link_libraries(tula_test
    PRIVATE
        tula::tula
        tula::testing
    )

add_dependencies(check tula_test)
gtest_discover_tests(tula_test TEST_PREFIX "tula::")
