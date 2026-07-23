#include <tula_boilerplate/config.h>

#include <iostream>

#if TULA_BOILERPLATE_HAS_LOGGING
#include <spdlog/spdlog.h>
#endif

int main()
{
    std::cout << "tula_boilerplate " << TULA_BOILERPLATE_VERSION << '\n';
    std::cout << "logging provider: " << TULA_BOILERPLATE_LOGGING_PROVIDER << '\n';
#if TULA_BOILERPLATE_HAS_LOGGING
    spdlog::info("normalized tula::logging target is active");
#else
    std::cout << "logging feature is intentionally disabled\n";
#endif
    return 0;
}
