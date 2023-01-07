#pragma once

#include "../meta.h"
#include "core.h"
#include <tula/formatter/container.h>
#include <unordered_map>

namespace tula::nddata {

template <typename Derived>
struct LabelMapper {
    using index_t = typename tula::nddata::type_traits<Derived>::index_t;
    using label_t = typename tula::nddata::type_traits<Derived>::label_t;
    using labels_t = std::vector<label_t>;
    LabelMapper() = default;

    LabelMapper(labels_t labels_) noexcept
        : m_labels(std::move(labels_)), m_label_index{
                                            make_label_index(this->m_labels)} {}

    auto size() const noexcept -> index_t {
        return tula::meta::size_cast<index_t>(m_labels.size());
    }
    auto empty() const noexcept -> bool {
        return size() == 0;
    }
    auto index(const label_t &label) const -> index_t {
        auto it = m_label_index.find(label);
        if (it != m_label_index.end()) {
            return it->second;
        }
        throw std::runtime_error(
            fmt::format("label {} not found in {}", label, m_labels));
    }
    auto label(index_t i) const -> const label_t & {
        return this->labels().at(tula::meta::size_cast<std::size_t>(i));
    }
    auto labels() const noexcept -> const labels_t & { return m_labels; }
    auto operator()() const noexcept -> const auto & { return this->labels(); }

private:
    labels_t m_labels;
    using label_index_t = std::unordered_map<label_t, index_t>;
    label_index_t m_label_index;

    static auto make_label_index(const labels_t &labels) -> label_index_t {
        label_index_t label_index;
        for (std::size_t i = 0; i < labels.size(); ++i) {
            label_index.emplace(labels[i], i);
        }
        return label_index;
    }
};

} // namespace tula::nddata
