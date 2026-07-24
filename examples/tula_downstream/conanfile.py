from conan import ConanFile
from conan.tools.cmake import cmake_layout


class TulaDownstreamRecipe(ConanFile):
    name = "tula-downstream"
    version = "3.1.0"
    package_type = "application"
    required_conan_version = ">=2.31"
    settings = "os", "arch", "compiler", "build_type"
    requires = "tula-boilerplate/3.1.0"
    generators = "CMakeDeps", "CMakeToolchain"
    exports_sources = "CMakeLists.txt", "src/*"

    def layout(self) -> None:
        cmake_layout(self)
