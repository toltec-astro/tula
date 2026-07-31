#include <tula/config.h>
#include <tula_perflibs/config.h>

#include <thread>

#if TULA_PERFLIBS_HAS_OPENMP
#include <omp.h>
#endif

static_assert(TULA_HAS_PERFLIBS == 1);
static_assert(TULA_PERFLIBS_HAS_THREADS == 1);
static_assert(TULA_HAS_OPENMP == TULA_PERFLIBS_HAS_OPENMP);

#if TULA_PERFLIBS_HAS_OPENMP && !defined(_OPENMP)
#error "The enabled perflibs target did not propagate OpenMP compiler flags"
#endif

int main()
{
    auto worker = std::jthread([] {});
    worker.join();

#if TULA_PERFLIBS_HAS_OPENMP
    return omp_get_max_threads() > 0 ? 0 : 1;
#else
    return 0;
#endif
}
