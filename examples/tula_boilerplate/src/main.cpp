#include <tula_boilerplate/boilerplate.h>

#include <iostream>

#if TULA_BOILERPLATE_HAS_FORMATTING
#include <fmt/format.h>
#endif

#if TULA_BOILERPLATE_HAS_LOGGING
#include <spdlog/spdlog.h>
#endif

int main()
{
    std::cout << "tula_boilerplate " << tula_boilerplate::version << '\n';
    std::cout << "formatting provider: " << tula_boilerplate::formatting_provider << '\n';
    std::cout << "logging provider: " << tula_boilerplate::logging_provider << '\n';
#if TULA_BOILERPLATE_HAS_FORMATTING
    std::cout << fmt::format("formatting target: {}", "active") << '\n';
#endif
#if TULA_BOILERPLATE_HAS_LOGGING
    spdlog::info("normalized tula::logging target is active");
#else
    std::cout << "logging feature is intentionally disabled\n";
#endif
    return 0;
}
