project (tula_test LANGUAGES CXX C)

conan_cmake_autodetect(settings)
conan_cmake_configure(REQUIRES
    benchmark/[>=1.5]
    gtest/cci.20210126
    GENERATORS cmake_find_package)
conan_cmake_install(PATH_OR_REFERENCE ${CMAKE_BINARY_DIR}
                    BUILD outdated
                    REMOTE conancenter
                    SETTINGS ${settings}
                    INSTALL_FOLDER ${CMAKE_BINARY_DIR})
find_package(benchmark)
find_package(GTest)

enable_testing()

# https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/EmulateMakeCheck
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
include(GoogleTest)

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
        tula::all
        GTest::gtest GTest::gmock benchmark::benchmark
    )

add_dependencies(check tula_test)
gtest_discover_tests(tula_test TEST_PREFIX "tula::")
