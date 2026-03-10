cmake_minimum_required(VERSION 4.1)

project(mpi
    DESCRIPTION "tula v3 example: MPI with grppi parallel patterns"
    LANGUAGES CXX C
)

add_executable(mpi)
target_sources(mpi PRIVATE main.cpp)
target_link_libraries(mpi
    PRIVATE
        tula::headers
        tula::logging
        tula::MXX
        tula::Grppi
        tula::Eigen3
)
target_compile_features(mpi PRIVATE cxx_std_23)
