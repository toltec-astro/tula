#include <tula/nc.h>

#include <cstdint>
#include <filesystem>
#include <string>
#include <type_traits>

int main()
{
    using namespace tula::nc_utils;
    using NcClass = netCDF::NcType::ncType;

    static_assert(
        std::is_same_v<internal::type_t<NcClass::nc_CHAR>, char>);
    static_assert(
        std::is_same_v<internal::type_t<NcClass::nc_INT>, std::int32_t>);
    static_assert(internal::nctype_v<double> == NcClass::nc_DOUBLE);

    const auto path =
        std::filesystem::temp_directory_path() / "tula-nc-behavior.nc";
    std::filesystem::remove(path);
    {
        auto file = netCDF::NcFile(path.string(), netCDF::NcFile::replace);
        auto count = file.addVar("count", netCDF::ncInt);
        const std::int32_t output = 42;
        count.putVar(&output);

        if (getscalar<std::int32_t>(count) != output) {
            return 1;
        }
        const auto visited = visit(
            [](const auto &, auto value) {
                return std::is_same_v<decltype(value), std::int32_t>;
            },
            count);
        if (!visited) {
            return 2;
        }
        if (pprint{count}.str().find("count") == std::string::npos ||
            pprint{file}.str().find("n_vars: 1") == std::string::npos) {
            return 3;
        }
    }
    std::filesystem::remove(path);
    return 0;
}
