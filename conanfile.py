import sys
from pathlib import Path

# Import TulaConan base class from tula_cmake
sys.path.insert(0, str(Path(__file__).parent / "tula_cmake"))
from tula_conan import TulaConan

class TulaRecipe(TulaConan):
    """
    Tula package recipe using the v3 Conan-centric architecture.
    
    This is a simple one-line subclass - all the magic happens in TulaConan base class!
    
    Usage:
        # Enable packages with mode selection via Conan options
        conan install . -o "&:Eigen3=CONAN"       # Use Conan package
        conan install . -o "&:Eigen3=CPM"         # Fetch via CPM
        conan install . -o "&:Eigen3=AUTO"        # Try Conan, fallback to CPM/SYSTEM
        conan install . -o "&:logging=CONAN"      # Metapackage (spdlog + fmt)
        conan install . -o "&:testing=CPM"        # Metapackage (gtest + benchmark)
        
        # Multiple packages
        conan install . -o "&:Eigen3=CONAN" -o "&:Yaml=CPM" -o "&:logging=AUTO"
    
    The TulaConan base class:
    - Discovers all packages from tula_cmake/targets/*.py
    - Generates options dynamically
    - Adds Conan requirements for AUTO/CONAN modes
    - Generates CMakeToolchain with per-package configuration blocks
    - Implements lazy evaluation (packages loaded only when used)
    """
    pass  # That's it! All functionality inherited from TulaConan


# Note: For backwards compatibility during migration, you can override methods:
#
# class TulaRecipe(TulaConan):
#     def requirements(self):
#         super().requirements()
#         # Add any tula-specific requirements
#         pass
#     
#     def build(self):
#         from conan.tools.cmake import CMake
#         cmake = CMake(self)
#         cmake.configure()
#         cmake.build()

