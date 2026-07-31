#include <tula/config.h>
#include <tula_perflibs/config.h>

#if TULA_PERFLIBS_HAS_OPENMP
#include <omp.h>
#endif

static_assert(TULA_HAS_PERFLIBS == 1);
static_assert(TULA_HAS_OPENMP == EXPECT_OPENMP);
static_assert(TULA_PERFLIBS_HAS_OPENMP == EXPECT_OPENMP);

#if EXPECT_OPENMP && !defined(_OPENMP)
#error "Installed tula::perflibs did not propagate OpenMP"
#endif

int main()
{
#if TULA_PERFLIBS_HAS_OPENMP
    return omp_get_max_threads() > 0 ? 0 : 1;
#else
    return 0;
#endif
}
