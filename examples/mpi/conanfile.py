from tula_cmake import TulaConan


class MpiRecipe(TulaConan):
    """
    mpi: Example demonstrating tula MPI + grppi parallel patterns.

    Demonstrates:
    - tula::MXX (MPI C++ wrapper, system MPI) — NOTE: MXX not yet in tula_cmake targets
    - tula::Grppi (parallel execution policies)
    - tula::Eigen3 (linear algebra)
    - tula::logging (spdlog/fmt)

    Prerequisites:
        System MPI must be installed (e.g., libopenmpi-dev on Ubuntu).

    Usage:
        uv run conan install . --profile=profiles/clang20-debug --build=missing
        cmake --preset conan-clang20-debug
        cmake --build build/clang20-debug
        mpirun -np 4 ./build/clang20-debug/mpi
    """
    pass
