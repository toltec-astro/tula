#include <netcdf>
#include <tula/config.h>

int main()
{
    return TULA_VERSION[0] == '\0' || sizeof(netCDF::NcFile) == 0;
}
