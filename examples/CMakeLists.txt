add_custom_target(test_progs)
foreach(prog mpi cli_builder)
    add_subdirectory(${prog})
endforeach()
