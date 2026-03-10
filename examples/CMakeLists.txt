# tula/examples/CMakeLists.txt
#
# Each example is a self-contained v3 project. Primary usage is standalone:
#
#   cd examples/tula_boilerplate
#   uv run conan install . --profile=profiles/clang20-debug --build=missing
#   cmake --preset conan-clang20-debug && cmake --build build/clang20-debug
#
# When included via TULA_BUILD_EXAMPLES=ON, examples are built as sub-projects
# using the parent tula build context (tula::headers and tula::tula available).

foreach(example tula_boilerplate cli_builder ecsv_reader mpi)
    add_subdirectory(${example})
endforeach()
