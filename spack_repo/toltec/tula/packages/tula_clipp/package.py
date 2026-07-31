"""Header-only Clipp fork used by TolTEC."""

from spack_repo.builtin.build_systems.generic import Package
from spack.package import depends_on, install_tree, join_path, mkdirp, patch, version


class TulaClipp(Package):
    """Install pinned Clipp headers with the C++20 invoke-result fix."""

    homepage = "https://github.com/GerHobbelt/clipp"
    git = "https://github.com/GerHobbelt/clipp.git"

    version(
        "2021.05.05",
        commit="ddf69f70eaaefe318cc8aa0d018ff523111410bb",
    )

    patch("cxx20-invoke-result.patch")

    depends_on("cxx", type="build")

    def install(self, spec, prefix) -> None:
        install_tree("include", prefix.include)
        config_dir = join_path(prefix.lib, "cmake", "TulaClipp")
        mkdirp(config_dir)
        with open(join_path(config_dir, "TulaClippConfig.cmake"), "w") as stream:
            stream.write(
                "get_filename_component(_prefix "
                '"${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)\n'
                "add_library(tula_deps::clipp INTERFACE IMPORTED)\n"
                "set_target_properties(tula_deps::clipp PROPERTIES "
                'INTERFACE_INCLUDE_DIRECTORIES "${_prefix}/include")\n'
            )
