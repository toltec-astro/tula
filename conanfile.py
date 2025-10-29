from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps

# Define all available dependencies
# Option names match the CMake package files in tula_cmake/cmake/targets/
TULA_DEPENDENCIES = [
    # Core utilities
    {"option": "Eigen3", "package": "eigen", "version": "[>=3.4 <5.0]"},
    {"option": "fmt", "package": "fmt", "version": "[>=10.0 <12.0]"},
    {"option": "spdlog", "package": "spdlog", "version": "[>=1.13 <2.0]"},
    {"option": "Yaml", "package": "yaml-cpp", "version": "[>=0.8 <1.0]"},
    
    # Scientific computing
    {"option": "Ceres", "package": "ceres-solver", "version": "[>=2.2 <3.0]"},
    {"option": "FFTW", "package": "fftw", "version": "[>=3.3 <4.0]"},
    {"option": "Spectra", "package": "spectra", "version": "[>=1.0 <2.0]"},
    
    # I/O libraries
    {"option": "NetCDF", "package": "netcdf-c", "version": "[>=4.9 <5.0]"},
    {"option": "NetCDFCXX4", "package": "netcdf-cxx4", "version": "[>=4.3 <5.0]"},
    {"option": "CCfits", "package": "ccfits", "version": "[>=2.5 <3.0]"},
    
    # Other utilities
    {"option": "Re2", "package": "re2", "version": "[>=20240101 <20260101]"},
    
    # Testing and utilities
    {"option": "gtest", "package": "gtest", "version": "[>=1.14 <2.0]"},
    {"option": "benchmark", "package": "benchmark", "version": "[>=1.8 <2.0]"},
    # Not available in Conan Center: MXX (requires MPI)
]

class TulaRecipe(ConanFile):
    name = "tula"
    version = "2.0"
    
    # Settings
    settings = "os", "compiler", "build_type", "arch"
    
    # Generate options and default_options from TULA_DEPENDENCIES
    # Option names match CMake package names in tula_cmake/cmake/targets/*.cmake
    # 
    # Usage:
    #   When installing tula directly (conan install . in tula directory):
    #     -o "&:Eigen3=True" -o "&:Yaml=True"              # Self-reference (cleanest)
    #     -o "tula/*:Eigen3=True" -o "tula/*:Yaml=True"    # Explicit scope
    # 
    #   When tula is a dependency of downstream package:
    #     -o "tula/*:Eigen3=True" -o "tula/*:Yaml=True"    # MUST use explicit scope
    #     (Cannot use "&:" as it refers to the consumer, not tula)
    # 
    # Note: CPM fetches don't use conan options - these only apply when tula is
    # consumed via conan (e.g., conan install, not CPMAddPackage)
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

