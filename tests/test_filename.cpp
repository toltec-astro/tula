#include <tula/filename.h>

#include <filesystem>
#include <fstream>
#include <string>

int main()
{
    namespace fs = std::filesystem;
    using namespace tula::filename_utils;

    const auto parsed =
        parse_pattern("{stem}{index}.txt", "observation.nc", fmt::arg("index", 2));
    if (fs::path(parsed).filename() != "observation2.txt") {
        return 1;
    }

    const auto root =
        fs::temp_directory_path() / "tula-filename-behavior-test";
    fs::remove_all(root);
    const auto created = fs::path(create_dir_if_not_exist(root.string()));
    if (!fs::is_directory(created)) {
        return 2;
    }

    std::ofstream{created / "toltec0.nc"} << "test";
    std::ofstream{created / "ignore.txt"} << "test";
    const auto matches = find_regex(created.string(), R"(toltec[0-9]+\.nc)");
    fs::remove_all(root);
    return matches.size() == 1 &&
                   fs::path(matches.front()).filename() == "toltec0.nc"
               ? 0
               : 3;
}
