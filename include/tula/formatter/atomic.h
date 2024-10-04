#pragma once
#include <fmt/core.h>
#include <atomic>



namespace fmt {


template <typename T>
struct formatter<std::atomic<T>>: formatter<T> {

  auto format(const std::atomic<T> & v, format_context& ctx) const
    -> format_context::iterator {
	return fmt::formatter<T>::format(v.load(), ctx);
    }
};

} // namespace fmt
