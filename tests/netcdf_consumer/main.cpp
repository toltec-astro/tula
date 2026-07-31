#include <tula/config.h>
#include <tula/nc.h>

#include <cstdint>
#include <filesystem>

int main()
{
    static_assert(TULA_HAS_NETCDF == 1);
    const auto path =
        std::filesystem::temp_directory_path() / "tula-netcdf-consumer.nc";
    std::filesystem::remove(path);
    {
        auto file = netCDF::NcFile(path.string(), netCDF::NcFile::replace);
        auto value = file.addVar("value", netCDF::ncInt);
        const std::int32_t expected = 17;
        value.putVar(&expected);
        if (tula::nc_utils::getscalar<std::int32_t>(value) != expected) {
            return 1;
        }
    }
    std::filesystem::remove(path);
    return 0;
}
