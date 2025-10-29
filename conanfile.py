from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps

# Define all available dependencies
TULA_DEPENDENCIES = [
    # Core utilities
    {"option": "eigen3", "package": "eigen", "version": "[>=3.4 <5.0]"},
    {"option": "fmt", "package": "fmt", "version": "[>=10.0 <12.0]"},
    {"option": "spdlog", "package": "spdlog", "version": "[>=1.13 <2.0]"},
    {"option": "yaml_cpp", "package": "yaml-cpp", "version": "[>=0.8 <1.0]"},
    
    # Scientific computing
    {"option": "ceres", "package": "ceres-solver", "version": "[>=2.2 <3.0]"},
    {"option": "fftw", "package": "fftw", "version": "[>=3.3 <4.0]"},
    {"option": "spectra", "package": "spectra", "version": "[>=1.0 <2.0]"},
    
    # I/O libraries
    {"option": "netcdf", "package": "netcdf-c", "version": "[>=4.9 <5.0]"},
    {"option": "netcdf_cxx4", "package": "netcdf-cxx4", "version": "[>=4.3 <5.0]"},
    {"option": "ccfits", "package": "ccfits", "version": "[>=2.5 <3.0]"},
    
    # Other utilities
    {"option": "re2", "package": "re2", "version": "[>=20240101 <20260101]"},
    
    # Testing (not available in Conan Center: mxx)
    {"option": "gtest", "package": "gtest", "version": "[>=1.14 <2.0]"},
    {"option": "benchmark", "package": "benchmark", "version": "[>=1.8 <2.0]"},
]

class TulaRecipe(ConanFile):
    name = "tula"
    version = "2.0"
    
    # Settings
    settings = "os", "compiler", "build_type", "arch"
    
    # Generate options and default_options from TULA_DEPENDENCIES
    options = {dep["option"]: [True, False] for dep in TULA_DEPENDENCIES}
    default_options = {dep["option"]: False for dep in TULA_DEPENDENCIES}
    
    def requirements(self):
        """Add Conan dependencies when enabled - Conan handles transitive deps automatically"""
        for dep in TULA_DEPENDENCIES:
            if getattr(self.options, dep["option"]):
                self.requires(f"{dep['package']}/{dep['version']}")
    
    def generate(self):
        """Generate CMake integration files"""
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        # Disable CMake presets generation - we have our own CMakePresets.json
        tc.user_presets_path = False
        tc.generate()

