from tula_cmake import TulaConan


class EcsvReaderRecipe(TulaConan):
    """
    ecsv_reader: Example demonstrating tula ECSV table reading.

    Demonstrates:
    - tula::ecsv (Enhanced CSV / Astropy ECSV format)
    - tula::Csv (aria-csv parser, enabled via *:Csv=cpm)
    - tula::Eigen3 (matrix output of table columns)
    - tula::cli (clipp-based CLI)
    - tula::logging (spdlog/fmt)
    - tula::Yaml (yaml-cpp for ECSV metadata)

    Usage:
        uv run conan install . --profile=profiles/clang20-debug --build=missing
        cmake --preset conan-clang20-debug
        cmake --build build/clang20-debug
        ./build/clang20-debug/ecsv_reader <path/to/file.ecsv>
    """
    pass
