#include <tula/ecsv/table.h>

#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr auto ecsv_text = R"(# %ECSV 0.9
# ---
# datatype:
# - {name: uid, datatype: string}
# - {name: count, datatype: int32}
# - {name: value, unit: GHz, datatype: float64}
# schema: astropy-2.0
uid count value
)";

} // namespace

int main()
{
    using namespace tula::ecsv;

    std::stringstream content;
    content << ecsv_text;
    const auto header = ECSVHeader::read(content);

    const auto numeric_view = ECSVHeaderView(
        header,
        [](const auto &column) { return column.datatype != "string"; });
    if (numeric_view.size() != 2 || numeric_view.col("value").unit != "GHz") {
        return 1;
    }

    auto table = ECSVTable(header);
    std::vector<std::vector<std::string>> rows{
        {"detector-1", "2", "1.25"},
        {"detector-2", "4", "2.50"},
    };
    table.load_rows(rows);

    if (table.rows() != 2 || table.cols() != 3) {
        return 2;
    }
    if (table.col<std::string>("uid")(0) != "detector-1") {
        return 3;
    }
    if (table.col<int32_t>("count")(1) != 4) {
        return 4;
    }
    if (table.col<double>("value")(1) != 2.5) {
        return 5;
    }
    return table.info().find("float64") == std::string::npos ? 6 : 0;
}
