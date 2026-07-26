from conan import ConanFile
from conan.tools.cmake import CMake


class TulaRecipe(ConanFile):
    name = "tula"
    version = "3.1.0"
    description = "Shared C++ utility headers for TolTEC"
    license = "BSD-3-Clause"
    url = "https://github.com/toltec-astro/tula"
    package_type = "header-library"
    required_conan_version = ">=2.31"
    python_requires = "tula-cmake/3.1.0"
    python_requires_extend = "tula-cmake.TulaConan"
    settings = ()
    options = {}
    default_options = {}
    tula_default_options = {
        "logging": "conan",
        "yaml_cpp": "conan",
        "csv_parser": "cpm",
        "netcdf_c": "system",
        "netcdf_cxx4": "system",
        "bitmask": "cpm",
        "meta_enum": "cpm",
        "clipp": "conan",
        "perflibs": "system",
        "eigen": "conan",
        "grppi": "cpm",
    }
    tula_public_features = tuple(tula_default_options)
    exports_sources = "CMakeLists.txt", "include/*", "tests/*"

    def build(self) -> None:
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self) -> None:
        CMake(self).install()

    def package_info(self) -> None:
        self.cpp_info.set_property("cmake_file_name", "tula")
        self.cpp_info.set_property("cmake_target_name", "tula::headers")
