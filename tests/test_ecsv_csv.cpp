#include <csv_parser/parser.hpp>
#include <tula/ecsv/table.h>

#include <sstream>
#include <string>

namespace {

constexpr auto ecsv_text = R"(# %ECSV 0.9
# ---
# delimiter: ';'
# datatype:
# - {name: uid, datatype: string}
# - {name: count, datatype: int32}
# - {name: value, unit: GHz, datatype: float64}
# schema: astropy-2.0
uid;count;value
"detector;1";2;1.25
detector-2;4;2.50
)";

} // namespace

int main()
{
    using namespace tula::ecsv;

    std::istringstream content{ecsv_text};
    const auto header = ECSVHeader::read(content);
    auto parser = aria::csv::CsvParser(content).delimiter(header.delimiter());
    auto table = ECSVTable(header);
    table.load_rows(parser);

    if (table.rows() != 2 || table.cols() != 3) {
        return 1;
    }
    if (table.col<std::string>("uid")(0) != "detector;1") {
        return 2;
    }
    if (table.col<int32_t>("count")(1) != 4) {
        return 3;
    }
    return table.col<double>("value")(1) == 2.5 ? 0 : 4;
}
