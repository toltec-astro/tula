from conan import ConanFile


class TulaRecipe(ConanFile):
    name = "tula"
    version = "3.1.0"
    package_type = "header-library"
    required_conan_version = ">=2.31"
    python_requires = "tula-cmake/3.1.0"
    python_requires_extend = "tula-cmake.TulaConan"
    settings = ()
    options = {}
    default_options = {}
    exports_sources = "CMakeLists.txt", "include/*", "tests/*"
