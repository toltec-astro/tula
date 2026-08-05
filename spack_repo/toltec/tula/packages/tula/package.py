"""Spack package for the TolTEC C++ utility modules."""

from spack.package import (
    depends_on,
    on_package_attributes,
    requires,
    run_after,
    variant,
    version,
    working_dir,
)
from spack.util.executable import Executable
from spack_repo.builtin.build_systems.cmake import CMakePackage


class Tula(CMakePackage):
    """Install an explicitly selected closure of cohesive Tula modules."""

    homepage = "https://github.com/toltec-astro/tula"
    git = "https://github.com/toltec-astro/tula.git"

    version("3.1.0", commit="212717a2844fe1da7c4248dfefdead2ff21e80be")

    for feature, description in (
        ("logging", "logging and fmt-based formatting"),
        ("yaml", "YAML-backed configuration"),
        ("ecsv", "complete ECSV typed-table support"),
        ("eigen", "Eigen helpers and numeric algorithms"),
        ("netcdf", "NetCDF C++ I/O helpers"),
        ("enum", "reflected enums and typed flags"),
        ("cli", "command-line construction"),
        ("perflibs", "Threads and optional OpenMP"),
        ("grppi", "parallel-pattern helpers"),
        ("fitting", "Ceres-backed fitting helpers"),
    ):
        variant(feature, default=False, description=f"Enable {description}")
    variant(
        "openmp",
        default=False,
        description="Expose OpenMP through perflibs and GrPPI",
    )

    for required in ("logging", "yaml", "eigen"):
        requires(f"+{required}", when="+ecsv")
    requires("+logging", when="+yaml")
    requires("+logging", when="+eigen")
    requires("+eigen", when="+netcdf")
    requires("+logging", when="+enum")
    for required in ("logging", "enum"):
        requires(f"+{required}", when="+cli")
    requires("+perflibs", when="+openmp")
    for required in ("logging", "enum", "perflibs"):
        requires(f"+{required}", when="+grppi")
    for required in ("logging", "eigen"):
        requires(f"+{required}", when="+fitting")

    depends_on("cmake@3.25:", type="build")
    depends_on("cxx", type="build")
    depends_on("tula-cmake@3.2.0", type="build")
    depends_on("tula-logging@1", when="+logging", type=("build", "link"))
    depends_on("tula-yaml-cpp@0.8.0", when="+yaml", type=("build", "link"))
    depends_on(
        "tula-csv-parser@2020.06.12",
        when="+ecsv",
        type=("build", "link"),
    )
    depends_on("tula-eigen3@3.4.0", when="+eigen", type=("build", "link"))
    depends_on(
        "tula-netcdf-cxx4@4.3.1",
        when="+netcdf",
        type=("build", "link"),
    )
    depends_on("tula-bitmask", when="+enum", type=("build", "link"))
    depends_on("tula-meta-enum", when="+enum", type=("build", "link"))
    depends_on("tula-clipp", when="+cli", type=("build", "link"))
    depends_on(
        "tula-perflibs+openmp",
        when="+perflibs+openmp",
        type=("build", "link"),
    )
    depends_on(
        "tula-perflibs~openmp",
        when="+perflibs~openmp",
        type=("build", "link"),
    )
    depends_on("tula-grppi", when="+grppi", type=("build", "link"))
    depends_on("ceres-solver@2.2.0", when="+fitting", type=("build", "link"))

    def cmake_args(self) -> list[str]:
        """Map the concrete module closure to strict CMake switches."""
        mapping = {
            "logging": "TULA_ENABLE_LOGGING",
            "yaml": "TULA_ENABLE_YAML",
            "ecsv": "TULA_ENABLE_ECSV",
            "eigen": "TULA_ENABLE_EIGEN",
            "netcdf": "TULA_ENABLE_NETCDF",
            "enum": "TULA_ENABLE_ENUM",
            "cli": "TULA_ENABLE_CLI",
            "perflibs": "TULA_ENABLE_PERFLIBS",
            "openmp": "TULA_ENABLE_OPENMP",
            "grppi": "TULA_ENABLE_GRPPI",
            "fitting": "TULA_ENABLE_FITTING",
        }
        args = [
            self.define_from_variant(cmake_name, variant_name)
            for variant_name, cmake_name in mapping.items()
        ]
        args.append(self.define("TULA_BUILD_TESTS", self.run_tests))
        args.extend(
            [
                self.define("TULA_PACKAGE_SPEC", str(self.spec)),
                self.define("TULA_DAG_HASH", self.spec.dag_hash()),
            ]
        )
        return args

    @run_after("build")
    @on_package_attributes(run_tests=True)
    def check(self) -> None:
        """Run every enabled module test during ``spack install --test``."""
        with working_dir(self.build_directory):
            ctest = Executable("ctest")
            listing = ctest("-N", output=str)
            if "Total Tests: 0" in listing:
                raise RuntimeError("Tula configured without tests")
            ctest("--output-on-failure")
