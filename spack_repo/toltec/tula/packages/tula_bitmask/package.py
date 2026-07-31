"""Header-only bitmask package used by TolTEC."""

from spack_repo.builtin.build_systems.generic import Package
from spack.package import depends_on, install_tree, join_path, mkdirp, version


class TulaBitmask(Package):
    """Install pinned bitmask headers and an imported CMake target."""

    homepage = "https://github.com/oliora/bitmask"
    url = "https://github.com/oliora/bitmask/archive/0454f32733d4fc910ac0c3c85e61b45b9ae7eee9.tar.gz"

    version(
        "2019.02.19",
        url="https://github.com/oliora/bitmask/archive/0454f32733d4fc910ac0c3c85e61b45b9ae7eee9.tar.gz",
        sha256="5fa767a9bf78948ff50c056469a35b2f50e41dec6b7e1b15ca6372cb5403a317",
    )

    depends_on("cxx", type="build")

    def install(self, spec, prefix) -> None:
        install_tree("include", prefix.include)
        config_dir = join_path(prefix.lib, "cmake", "TulaBitmask")
        mkdirp(config_dir)
        with open(join_path(config_dir, "TulaBitmaskConfig.cmake"), "w") as stream:
            stream.write(
                "get_filename_component(_prefix "
                '"${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)\n'
                "add_library(tula_deps::bitmask INTERFACE IMPORTED)\n"
                "set_target_properties(tula_deps::bitmask PROPERTIES "
                'INTERFACE_INCLUDE_DIRECTORIES "${_prefix}/include")\n'
            )
