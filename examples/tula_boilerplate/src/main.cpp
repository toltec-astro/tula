#include <tula_boilerplate/boilerplate.h>

#include <iostream>

#if TULA_BOILERPLATE_HAS_LOGGING
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#endif

int main()
{
    std::cout << "tula_boilerplate " << tula_boilerplate::version << '\n';
    std::cout << "logging provider: " << tula_boilerplate::logging_provider << '\n';
#if TULA_BOILERPLATE_HAS_LOGGING
    std::cout << fmt::format("fmt supplied by logging: {}", "active") << '\n';
    spdlog::info("normalized tula::logging target is active");
#else
    std::cout << "logging feature is intentionally disabled\n";
#endif
    return 0;
}
