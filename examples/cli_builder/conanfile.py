from tula_cmake import TulaConan


class CliBuiderRecipe(TulaConan):
    """
    cli_builder: Example demonstrating tula CLI + config + grppi patterns.

    Demonstrates:
    - tula::cli (clipp-based CLI builder)
    - tula::config (YAML + flat config)
    - tula::logging (spdlog/fmt)
    - tula::Grppi (parallel execution policies)
    - tula::Enum (meta_enum)
    - tula::Yaml (yaml-cpp)

    Usage:
        uv run conan install . --profile=profiles/clang20-debug --build=missing
        cmake --preset conan-clang20-debug
        cmake --build build/clang20-debug
        ./build/clang20-debug/cli_builder --help
    """
    pass
