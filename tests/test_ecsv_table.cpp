#include <tula/ecsv/table.h>

#include <sstream>
#include <cmath>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>
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
    static_assert(std::is_lvalue_reference_v<
                  decltype(std::as_const(table).header_view())>);
    static_assert(
        std::is_lvalue_reference_v<decltype(std::as_const(table).header())>);
    static_assert(
        std::is_lvalue_reference_v<decltype(std::as_const(table).loader())>);
    std::vector<std::vector<std::string>> rows{
        {"detector 1", "2", "1.25"},
        {"detector-2", "4", "2.50"},
    };
    table.load_rows(rows);

    if (table.rows() != 2 || table.cols() != 3) {
        return 2;
    }
    if (table.col<std::string>("uid")(0) != "detector 1") {
        return 3;
    }
    if (table.col<int32_t>("count")(1) != 4) {
        return 4;
    }
    if (table.col<double>("value")(1) != 2.5) {
        return 5;
    }
    std::size_t column_count = 0;
    for ([[maybe_unused]] const auto &column : table.header_view().cols()) {
        ++column_count;
    }
    if (column_count != 3) {
        return 6;
    }
    if (table.info().find("float64") == std::string::npos) return 7;

    ECSVTable nonfinite(header);
    std::vector<std::vector<std::string>> special_rows{
        {"unknown amplitude", "0", "nan"}, {"upper", "1", "+Inf"},
        {"lower", "2", "-infinity"}};
    nonfinite.load_rows(special_rows);
    if (!std::isnan(nonfinite.col<double>("value")(0)) ||
        nonfinite.col<double>("value")(1) != std::numeric_limits<double>::infinity() ||
        nonfinite.col<double>("value")(2) != -std::numeric_limits<double>::infinity()) return 8;
    for (const auto &row : std::vector<std::vector<std::string>>{
            {"bad", "2x", "1"}, {"overflow", "2147483648", "1"},
            {"bad sign", "+-1", "1"},
            {"empty", "", "1"}, {"bad", "1", "1.2x"}, {"bad", "1", ""}}) {
        ECSVTable invalid(header);
        try {
            std::vector<std::vector<std::string>> invalid_rows{row};
            invalid.load_rows(invalid_rows);
            return 9;
        } catch (const std::runtime_error &) {}
    }
    // Astropy booleans and 8-bit integer cells must be values, not characters.
    if (!internal::parse_cell<bool>("True") || internal::parse_cell<bool>("False") ||
        internal::parse_cell<std::int8_t>("-128") != -128 ||
        internal::parse_cell<std::uint8_t>("255") != 255) return 10;
    for (auto cell : {"-1", "256", "+"}) {
        try {
            (void)internal::parse_cell<std::uint8_t>(cell);
            return 11;
        } catch (const std::invalid_argument &) {}
    }
    return 0;
}
