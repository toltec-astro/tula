"""Header-only GrPPI fork used by TolTEC."""

from spack_repo.builtin.build_systems.generic import Package
from spack.package import depends_on, install_tree, join_path, mkdirp, version


class TulaGrppi(Package):
    """Install pinned GrPPI headers and an imported CMake target."""

    homepage = "https://github.com/Jerry-Ma/grppi"
    url = "https://github.com/Jerry-Ma/grppi/archive/12f5c11b1a5ad4a283e313bd1966bfd6446b9e24.tar.gz"

    version(
        "2020.06.11",
        url="https://github.com/Jerry-Ma/grppi/archive/12f5c11b1a5ad4a283e313bd1966bfd6446b9e24.tar.gz",
        sha256="cdfd607573aa75272b5963d0142b90d6cbf684936f5ccc1451568669d88c2169",
    )

    depends_on("cxx", type="build")

    def install(self, spec, prefix) -> None:
        install_tree("include", prefix.include)
        config_dir = join_path(prefix.lib, "cmake", "TulaGrppi")
        mkdirp(config_dir)
        with open(join_path(config_dir, "TulaGrppiConfig.cmake"), "w") as stream:
            stream.write(
                "get_filename_component(_prefix "
                '"${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)\n'
                "add_library(tula_deps::grppi INTERFACE IMPORTED)\n"
                "set_target_properties(tula_deps::grppi PROPERTIES "
                'INTERFACE_INCLUDE_DIRECTORIES "${_prefix}/include")\n'
            )
