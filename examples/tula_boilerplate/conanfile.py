from conan import ConanFile
from conan.tools.cmake import CMake


class TulaBoilerplateRecipe(ConanFile):
    name = "tula-boilerplate"
    version = "3.1.0"
    package_type = "header-library"
    required_conan_version = ">=2.31"
    python_requires = "tula-cmake/3.1.0"
    python_requires_extend = "tula-cmake.TulaConan"
    settings = ()
    options = {}
    default_options = {}
    exports_sources = "CMakeLists.txt", "include/*", "src/*"

    def build(self) -> None:
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self) -> None:
        CMake(self).install()

    def package_info(self) -> None:
        self.cpp_info.set_property("cmake_file_name", "tula_boilerplate")
        self.cpp_info.set_property(
            "cmake_target_name",
            "tula_boilerplate::headers",
        )
