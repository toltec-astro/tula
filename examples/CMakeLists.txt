add_custom_target(test_progs)
foreach(prog mpi cli_builder ecsv_reader)
    add_subdirectory(${prog})
endforeach()
