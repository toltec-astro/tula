#include <cstdlib>
#include <mpi.h>
#include <tula/container.h>
#include <tula/eigen.h>
#include <tula/formatter/matrix.h>
#include <tula/grppi.h>
#include <tula/logging.h>

auto whoami() {
    int size;
    int rank;
    int namelen;
    int verlen;
    std::array<char, MPI_MAX_PROCESSOR_NAME> name{};
    std::array<char, MPI_MAX_LIBRARY_VERSION_STRING> ver{};
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(name.data(), &namelen);
    MPI_Get_library_version(ver.data(), &verlen);
    return std::make_tuple(size, rank, std::string(name.data(), namelen),
                           std::string(ver.data(), verlen));
}

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
int main(int /*argc*/, char * /*argv*/[]) {
    tula::logging::init(spdlog::level::trace);
    try {
        MPI_Init(nullptr, nullptr);
        auto [mpisize, mpirank, mpiproc, mpiver] = whoami();
        if (0 == mpirank) {
            SPDLOG_TRACE("MPI version: {}", mpiver);
        }
        SPDLOG_TRACE("MPI context: rank {}/{} proc {}", mpirank, mpisize,
                     mpiproc);
        // run a grppi reduce
        const auto n = 10;
        auto data = tula::container_utils::index(n);
        tula::eigen_utils::as_eigen(data).array() += mpirank;
        SPDLOG_TRACE("rank {}: reduce data{}", mpirank, data);
        auto sum = grppi::reduce(tula::grppi_utils::dyn_ex(), data, 0.,
                                 [](auto x, auto y) { return x + y; });
        SPDLOG_TRACE("rank {}: result {}", mpirank, sum);
        MPI_Finalize();
        return EXIT_SUCCESS;
    } catch (std::exception const &e) {
        SPDLOG_ERROR("abort: {}", e.what());
        return EXIT_FAILURE;
    }
}
