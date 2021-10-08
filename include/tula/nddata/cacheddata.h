#pragma once
#include <mutex>

namespace tula::nddata {

/// @brief Data object that evaluate only on first call.
template <typename DataType, typename Evaluator>
struct CachedData {

    CachedData() = default;
    CachedData(const CachedData &other)
        : data(other.data), initialized(other.initialized) {}
    CachedData(CachedData &&other) noexcept
        : data(std::move(other.data)), initialized(other.initialized) {}
    ~CachedData() = default;
    auto operator=(const CachedData &other) -> CachedData & {
        data = other.data;
        initialized = other.initialized;
        return *this;
    }
    auto operator=(CachedData &&other) noexcept -> CachedData & {
        data = std::move(other.data);
        initialized = other.initialized;
        return *this;
    }

    /// @brief Return the cached data
    template <typename T>
    auto operator()(T &parent) const -> const auto & {
        std::scoped_lock lock(m_mutex);
        if (!initialized) {
            data = Evaluator::evaluate(parent);
            initialized = true;
        }
        return data;
    }

    /// @brief Invalidate the cached data.
    void invalidate() const {
        std::scoped_lock lock(m_mutex);
        initialized = false;
    }

private:
    mutable DataType data;
    mutable bool initialized{false};
    mutable std::mutex m_mutex{};
};

} // namespace tula::nddata

#define TULA_CACHED_GETTER_DECL(name, return_type)                             \
                                                                               \
public:                                                                        \
    auto name() const->decltype(auto) { return m_##name(*this); }              \
    auto name()->decltype(auto) { return m_##name(*this); }                    \
    auto name##_invalidate() const noexcept->decltype(auto) {                  \
        m_##name.invalidate();                                                 \
        return *this;                                                          \
    };                                                                         \
    auto name##_invalidate() noexcept->decltype(auto) {                        \
        m_##name.invalidate();                                                 \
        return *this;                                                          \
    };                                                                         \
                                                                               \
private:                                                                       \
    tula::nddata::CachedData<return_type, name##_evaluator> m_##name {}
