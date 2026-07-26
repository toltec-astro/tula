#include <tula/ecsv/core.h>
#include <tula/ecsv/hdr.h>

#include <fmt/format.h>

#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr auto ecsv_text = R"(# %ECSV 0.9
# ---
# datatype:
# - {name: uid, datatype: string, description: Unique identifier}
# - {name: value, unit: GHz, datatype: float64}
# meta: {enabled: true, count: 2, label: sample}
# schema: astropy-2.0
uid value
detector-1 1.25
)";

auto check(bool condition, int error) -> int
{
    return condition ? 0 : error;
}

} // namespace

int main()
{
    using namespace tula::ecsv;

    std::stringstream content;
    content << ecsv_text;
    std::vector<std::string> processed;
    const auto [node, csv_header] = parse_header(content, &processed);

    if (const auto error = check(processed.front() == "# %ECSV 0.9", 1)) {
        return error;
    }
    if (const auto error =
            check(node["schema"].as<std::string>() == "astropy-2.0", 2)) {
        return error;
    }
    if (const auto error = check(csv_header == "uid value", 3)) {
        return error;
    }

    std::stringstream header_content;
    header_content << ecsv_text;
    const auto header = ECSVHeader::read(header_content);
    if (const auto error = check(header.size() == 2, 4)) {
        return error;
    }
    if (const auto error = check(header.cols()[0].name == "uid", 5)) {
        return error;
    }
    if (const auto error = check(header.cols()[1].unit == "GHz", 6)) {
        return error;
    }
    if (const auto error = check(header.schema() == "astropy-2.0", 7)) {
        return error;
    }
    if (const auto error = check(header.spec_version() == "0.9", 8)) {
        return error;
    }
    if (const auto error =
            check(fmt::format("{}", header) == "ECSVHeader(ncols=2)", 9)) {
        return error;
    }

    YAML::Node remaining;
    const auto bool_meta =
        meta_to_map<std::string, bool>(header.meta(), &remaining);
    if (const auto error = check(bool_meta.at("enabled"), 10)) {
        return error;
    }
    const auto int_meta = meta_to_map<std::string, int>(remaining, &remaining);
    if (const auto error = check(int_meta.at("count") == 2, 11)) {
        return error;
    }
    const auto string_meta =
        meta_to_map<std::string, std::string>(remaining, &remaining);
    return check(string_meta.at("label") == "sample", 12);
}
