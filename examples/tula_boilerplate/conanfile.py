from tula_cmake import TulaConan


class TulaBoilerplateRecipe(TulaConan):
    """
    tula_boilerplate: Minimum working example showing tula v3 integration.

    Demonstrates:
    - Eigen3 matrix operations
    - spdlog/fmt logging
    - yaml-cpp configuration
    - meta_enum / bitmask enum utilities
    - grppi parallel patterns

    Usage:
        uv run conan install . --profile=profiles/clang20-debug --build=missing
        cmake --preset conan-clang20-debug
        cmake --build build/clang20-debug
    """
    pass
