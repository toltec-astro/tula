"""Header-only meta_enum fork used by TolTEC."""

from spack_repo.builtin.build_systems.generic import Package
from spack.package import depends_on, install_tree, join_path, mkdirp, version


class TulaMetaEnum(Package):
    """Install pinned meta_enum headers and an imported CMake target."""

    homepage = "https://github.com/Jerry-Ma/meta_enum"
    url = "https://github.com/Jerry-Ma/meta_enum/archive/f940f15bc3f4321f5ef458742b7731c7f03543ff.tar.gz"

    version(
        "2020.06.15",
        url="https://github.com/Jerry-Ma/meta_enum/archive/f940f15bc3f4321f5ef458742b7731c7f03543ff.tar.gz",
        sha256="d44aceebba0dff499c3757da9678292db4b462c539677c7fac27ab0c84082e3d",
    )

    depends_on("cxx", type="build")

    def install(self, spec, prefix) -> None:
        install_tree("include", prefix.include)
        config_dir = join_path(prefix.lib, "cmake", "TulaMetaEnum")
        mkdirp(config_dir)
        with open(join_path(config_dir, "TulaMetaEnumConfig.cmake"), "w") as stream:
            stream.write(
                "get_filename_component(_prefix "
                '"${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)\n'
                "add_library(tula_deps::meta_enum INTERFACE IMPORTED)\n"
                "set_target_properties(tula_deps::meta_enum PROPERTIES "
                'INTERFACE_INCLUDE_DIRECTORIES "${_prefix}/include")\n'
            )
