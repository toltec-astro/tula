#pragma once

#include <tula_boilerplate/config.h>

#include <string_view>

namespace tula_boilerplate {

inline constexpr std::string_view version = TULA_BOILERPLATE_VERSION;
inline constexpr std::string_view logging_provider = TULA_BOILERPLATE_LOGGING_PROVIDER;

}  // namespace tula_boilerplate
