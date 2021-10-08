#include "common.h"
#include "tula/ecsv/core.h"
#include <fmt/ostream.h>
#include <gtest/gtest.h>
#include <ranges>
#include <sstream>
#include <tula/ecsv/table.h>
#include <tula/formatter/container.h>
#include <tula/formatter/matrix.h>
#include <yaml-cpp/node/emit.h>

namespace {

using namespace tula::testing;

// clang-format off
auto apt_header = R"apt_header(# %ECSV 0.9
# ---
# datatype:
# - name: uid
#   datatype: string
#   description: Unique id composed as "nw_pg_loc_ori".
#   meta: {uid_format: '{nw:02d}_{pg:1d}_{loc:03d}_{ori:1d}', uid_regex: '(?P<nw>\d{2})_(?P<pg>\d)_(?P<loc>\d{3})_(?P<ori>\d)'}
# - {name: nw, datatype: int64, description: Network index. Unique across all three arrays.}
# - {name: pg, datatype: int64, description: 'Polarization group. 0 for "+", and 1 for "-".'}
# - {name: loc, datatype: int64, description: Location index in polarization group.}
# - {name: ori, datatype: int64, description: 'Orientation index at given location. 0 is for the one with lower frequency, 1 is for the
#     other.'}
# - {name: fg, datatype: int64, description: 'Frequency group. From low frequency to hight the value goes as 0, 1, 2 and 3. Detectors
#     with fg 0 ("-") and 2 ("|") belongs to pg 0, and those with fg 1 ("\") and 3 ("/") belongs to pg 1.'}
# - {name: design_group, datatype: string, description: The original detector group label from the design spec sheet.}
# - {name: i, datatype: int64, description: The vertical sorted (row) index of the detector on the array grid.}
# - {name: j, datatype: int64, description: The horizontal sorted (column) index of the detector on the array grid.}
# - {name: k, datatype: int64, description: The per-network frequency-wise sorted index of the detector.}
# - {name: x, unit: um, datatype: int64, description: The x position designed.}
# - {name: y, unit: um, datatype: float64, description: The y position designed.}
# - {name: f, unit: GHz, datatype: float64, description: The frequency designed.}
# - name: flag
#   datatype: int64
#   description: State flag (0 - active; 1 - dark; 64 - invalid; 128 - unknown).
#   meta:
#     flag_values:
#       0: {description: The detector is active to optical power., label: active}
#       1: {description: The detector is not active to optical power., label: dark}
#       64: {description: This location dose not have a detector., label: invalid}
#       128: {description: Any case that is not covered by other bits., label: unknown}
# - {name: flag_summary, datatype: string, description: A summary of the set flag(s).}
# meta: !!omap
# - {name: a1100}
# - {name_long: TolTEC 1.1 mm array}
# - {index: 0}
# - {n_detectors: 4012}
# - wl_center: !astropy.units.Quantity
#     unit: !astropy.units.Unit {unit: cm}
#     value: 0.10999999999999999
# - grid:
#     description: The grid that embeds the detectors, with `x = (j - j0) * sj` and `y = (i - i0) * si`.
#     i0: 26.0
#     j0: 47.0
#     ni: 54
#     nj: 94
#     si: !astropy.units.Quantity
#       unit: &id001 !astropy.units.Unit {unit: um}
#       value: 2381.57
#     sj: !astropy.units.Quantity
#       unit: *id001
#       value: 1375.0
# - edge_indices: !numpy.ndarray
#     buffer: !!binary |
#       bUE4QUFBQUFBQUI0RHdBQUFBQUFBRklQQUFBQUFBQUFKZzhBQUFBQUFBRDBEZ0FBQUFBQUFMd09B
#       QUFBQUFBQWdBNEFBQUFBQUFCR0RnQUFBQUFBQUF3T0FBQUFBQUFBMEEwQUFBQUFBQUNRRFFBQUFB
#       QUFBRUlOQUFBQUFBQUE4Z3dBQUFBQUFBQ2dEQUFBQUFBQUFFd01BQUFBQUFBQTlnc0FBQUFBQUFD
#       ZUN3QUFBQUFBQUVRTEFBQUFBQUFBNkFvQUFBQUFBQUNPQ2dBQUFBQUFBRElLQUFBQUFBQUExQWtB
#       QUFBQUFBQjJDUUFBQUFBQUFCZ0pBQUFBQUFBQXVnZ0FBQUFBQUFCY0NBQUFBQUFBQVA0SEFBQUFB
#       QUFBb0FjQUFBQUFBQUJDQndBQUFBQUFBT1FHQUFBQUFBQUFoZ1lBQUFBQUFBQW9CZ0FBQUFBQUFN
#       b0ZBQUFBQUFBQWJBVUFBQUFBQUFBUUJRQUFBQUFBQUxZRUFBQUFBQUFBV2dRQUFBQUFBQUFBQkFB
#       QUFBQUFBS2dEQUFBQUFBQUFVZ01BQUFBQUFBRCtBZ0FBQUFBQUFLd0NBQUFBQUFBQVhBSUFBQUFB
#       QUFBT0FnQUFBQUFBQU00QkFBQUFBQUFBa2dFQUFBQUFBQUJZQVFBQUFBQUFBQjRCQUFBQUFBQUE0
#       Z0FBQUFBQUFBQ3FBQUFBQUFBQUFIWUFBQUFBQUFBQVNnQUFBQUFBQUFBaUFBQUFBQUFBQUFBQUFB
#       QUFBQUFBSUFBQUFBQUFBQUJJQUFBQUFBQUFBSFFBQUFBQUFBQUFxQUFBQUFBQUFBRGdBQUFBQUFB
#       QUFCd0JBQUFBQUFBQVZnRUFBQUFBQUFDUUFRQUFBQUFBQU13QkFBQUFBQUFBREFJQUFBQUFBQUJh
#       QWdBQUFBQUFBS29DQUFBQUFBQUEvQUlBQUFBQUFBQlFBd0FBQUFBQUFLWURBQUFBQUFBQS9nTUFB
#       QUFBQUFCWUJBQUFBQUFBQUxRRUFBQUFBQUFBRGdVQUFBQUFBQUJxQlFBQUFBQUFBTWdGQUFBQUFB
#       QUFKZ1lBQUFBQUFBQ0VCZ0FBQUFBQUFPSUdBQUFBQUFBQVFBY0FBQUFBQUFDZUJ3QUFBQUFBQVB3
#       SEFBQUFBQUFBV2dnQUFBQUFBQUM0Q0FBQUFBQUFBQllKQUFBQUFBQUFkQWtBQUFBQUFBRFNDUUFB
#       QUFBQUFEQUtBQUFBQUFBQWpBb0FBQUFBQUFEbUNnQUFBQUFBQUVJTEFBQUFBQUFBbkFzQUFBQUFB
#       QUQwQ3dBQUFBQUFBRW9NQUFBQUFBQUFuZ3dBQUFBQUFBRHdEQUFBQUFBQUFFQU5BQUFBQUFBQWpn
#       MEFBQUFBQUFET0RRQUFBQUFBQUFvT0FBQUFBQUFBUkE0QUFBQUFBQUIrRGdBQUFBQUFBTG9PQUFB
#       QUFBQUE4ZzRBQUFBQUFBQWtEd0FBQUFBQUFGQVBBQUFBQUFBQWRnOEFBQUFBQUFDV0R3QUFBQUFB
#       QUtvUEFBQUFBQUFB
#     dtype: int64
#     order: C
#     shape: !!python/tuple [108]
# - {generated_by: create_array_prop_table.py a1100}
# - {created_on: '2021-05-04T19:21:56.766'}
# - {version: v1.0.0}
# schema: astropy-2.0
uid nw pg loc ori fg design_group i j k x y f flag flag_summary
)apt_header";
// clang-format on

TEST(ecsv, parse_header) {

    using namespace tula::ecsv;

    std::stringstream ss;
    ss << apt_header;

    std::vector<std::string> processed;
    auto [ecsv_hdr, csv_hdr] = parse_header(ss, &processed);

    EXPECT_EQ(processed[0], "# %ECSV 0.9");
    EXPECT_EQ(ecsv_hdr["schema"].as<std::string>(), "astropy-2.0");
}

TEST(ecsv, hdr) {

    using namespace tula::ecsv;

    std::stringstream ss;
    ss << apt_header;

    std::vector<std::string> processed;
    auto hdr = ECSVHeader::read(ss, &processed);

    fmtlog("hdr={}", hdr);
    fmtlog("cols={}", hdr.cols());
    fmtlog("meta:\n{}", YAML::Dump(hdr.meta()));
    EXPECT_EQ(fmtlog("{}", hdr.schema()), "astropy-2.0");

    YAML::Node _meta{}; // this is to capture any uncaptured nodes
    const auto map_with_bools =
        meta_to_map<std::string, bool>(hdr.meta(), &_meta);
    const auto map_with_ints = meta_to_map<std::string, int>(_meta, &_meta);
    const auto map_with_strs =
        meta_to_map<std::string, std::string>(_meta, &_meta);

    fmtlog("meta(bool):{}", map_with_bools);
    fmtlog("meta(ints):{}", map_with_ints);
    fmtlog("meta(strs)):{}", map_with_strs);
    fmtlog("meta_rest:{}", YAML::Dump(_meta));

    // capture sumtype
    const auto map_with_variants =
        meta_to_map<std::string, std::variant<std::monostate, bool, int, double,
                                              std::string>>(hdr.meta());
    fmtlog("meta(var):{}", map_with_variants);

    // for data we know that is uniform, we can check agianst the type using
    // the follows
    decltype(auto) dtypes = hdr.datatypes();
    EXPECT_FALSE(check_uniform_dtype<int>(dtypes));
    EXPECT_FALSE(check_uniform_dtype<double>(dtypes));
}

TEST(ecsv, hdr_view) {

    using namespace tula::ecsv;

    std::stringstream ss;
    ss << apt_header;

    std::vector<std::string> processed;
    auto hdr = ECSVHeader::read(ss, &processed);
    auto hdrv = ECSVHeaderView(hdr);

    EXPECT_EQ(hdrv.col(0).name, hdr.cols()[0].name);
    EXPECT_EQ(hdrv.col("uid").name, hdr.cols()[0].name);

    auto hdrv2 = ECSVHeaderView(hdr, {"fg", "pg"});
    EXPECT_EQ(hdrv2.size(), 2);
    EXPECT_EQ(hdrv2.col("fg").name, hdr.cols()[5].name);
    EXPECT_EQ(hdrv2.colnames(), (std::vector<std::string>{"fg", "pg"}));
    EXPECT_EQ(hdrv2.cols()[1].name, "pg");
    EXPECT_NO_THROW(fmtlog("hdrv2{}", hdrv2));
    EXPECT_NO_THROW(fmtlog("colnames={}", hdrv2.colnames()));
    EXPECT_NO_THROW(fmtlog("datatypes={}", hdrv2.datatypes()));
}

TEST(ecsv, array_data) {

    using namespace tula::ecsv;
    std::stringstream ss;
    ss << apt_header;
    auto hdr = ECSVHeader::read(ss);

    // create hdrview for double type columns
    auto hdrv0 = ECSVHeaderView(hdr, [](const auto &col) {
        return (col.datatype == dtype_str<double>()) ||
               (col.datatype == dtype_str<float>());
    });
    // data container for holding the selected columns
    auto data0 = ArrayData<double>{hdrv0};
    static_assert(std::is_same_v<decltype(data0)::data_t, Eigen::ArrayXXd>);
    EXPECT_EQ(hdrv0.colnames(), (std::vector<std::string>{"y", "f"}));
    EXPECT_EQ(data0.row(100).size(), data0.size());
    // data container for holding the selected columns, using directly the
    // constructor
    auto data1 = ArrayData<int>{
        hdr, [](const auto &col) { return col.datatype.starts_with("int"); }};
    static_assert(std::is_same_v<decltype(data1)::data_t, Eigen::ArrayXXi>);
    EXPECT_EQ(data1.colnames(),
              (std::vector<std::string>{"nw", "pg", "loc", "ori", "fg", "i",
                                        "j", "k", "x", "flag"}));
    EXPECT_EQ(data1.row(3000).size(), data1.size());
    // data container for holding string types
    auto data2 = ArrayData<std::string>{hdr, [](const auto &col) {
                                            return col.datatype ==
                                                   dtype_str<std::string>();
                                        }};
    static_assert(std::is_same_v<decltype(data2)::data_t,
                                 std::vector<std::vector<std::string>>>);
    EXPECT_EQ(data2.colnames(), (std::vector<std::string>{"uid", "design_group",
                                                          "flag_summary"}));
    EXPECT_EQ(data2.row(0).size(), data2.size());
}

TEST(ecsv, dataloader) {

    using namespace tula::ecsv;
    std::stringstream ss;
    ss << apt_header;
    auto hdr = ECSVHeader::read(ss);

    auto data0 = ArrayData<double>{hdr, {"x", "y", "f", "nw"}};
    auto data1 = ArrayData<int>{hdr, {"nw", "pg", "loc", "ori", "fg"}};
    auto data2 = ArrayData<std::string>{hdr, [](const auto &col) {
                                            return col.datatype ==
                                                   dtype_str<std::string>();
                                        }};
    auto loader = ECSVDataLoader{hdr, data0, data1, data2};
    fmtlog("ref_index: {}", loader.get_ref_index());
    loader.ensure_row_size_for_index(2000);
    loader.visit_col("nw",
                     [](auto colref) { fmtlog("nw col: {}", colref.data); });
    loader.visit_col("nw", [](auto colref) { colref.set_value(0, 1); });
    loader.visit_col("uid",
                     [](auto colref) { colref.set_value(0, "some_value"); });
    loader.truncate(1);
    loader.visit_col("nw", [](auto colref) {
        fmtlog("col: {} {}", colref.col, colref.data);
    });

    fmtlog("data0{}", data0.array());
    fmtlog("data1{}", data1.array());
    fmtlog("data2{}", data2.array());
}

} // namespace
