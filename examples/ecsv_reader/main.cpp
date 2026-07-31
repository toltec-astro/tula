#include <csv_parser/parser.hpp>
#include <tula/ecsv/table.h>

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

auto read_tune_report(const std::filesystem::path &filepath) -> std::size_t
{
    std::ifstream input{filepath};
    if (!input) {
        throw std::runtime_error("unable to open " + filepath.string());
    }

    auto header = tula::ecsv::ECSVHeader::read(input);
    auto rows =
        aria::csv::CsvParser(input).delimiter(header.delimiter());
    auto table = tula::ecsv::ECSVTable(std::move(header));
    table.load_rows(rows);

    if (table.cols() != 14 || table.rows() == 0) {
        throw std::runtime_error(
            "unexpected table shape in " + filepath.string());
    }
    if (table.header_view().col(0).name != "f_out"
        || table.header_view().col(13).name != "interceptQ") {
        throw std::runtime_error(
            "unexpected tune-report columns in " + filepath.string());
    }
    if (!std::isfinite(table.col<double>("f_out")(0))
        || !std::isfinite(table.col<double>("Qr")(0))) {
        throw std::runtime_error(
            "non-finite tune data in " + filepath.string());
    }

    const auto metadata =
        tula::ecsv::meta_to_map<std::string, int>(table.header().meta());
    const auto obsnum = metadata.at("Header.Toltec.ObsNum");
    const auto subobsnum = metadata.at("Header.Toltec.SubObsNum");
    const auto scannum = metadata.at("Header.Toltec.ScanNum");
    if (obsnum <= 0 || subobsnum < 0 || scannum < 0) {
        throw std::runtime_error(
            "unexpected observation metadata in " + filepath.string());
    }

    std::cout << filepath.filename().string() << ": " << table.rows()
              << " rows, " << table.cols() << " columns, observation "
              << obsnum << '.' << subobsnum << '.' << scannum << '\n';
    return table.rows();
}

} // namespace

auto main(int argc, char *argv[]) -> int
{
    if (argc < 2) {
        std::cerr << "usage: tula_ecsv_reader TUNE_REPORT [TUNE_REPORT ...]\n";
        return EXIT_FAILURE;
    }

    try {
        std::size_t total_rows = 0;
        for (int index = 1; index < argc; ++index) {
            total_rows += read_tune_report(argv[index]);
        }
        std::cout << "loaded " << argc - 1 << " tune reports and "
                  << total_rows << " rows\n";
        return EXIT_SUCCESS;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
