#include "sxm/util/search_map/s.hpp"
#include <optional>

#pragma once

namespace sxm {

template <typename val_t>

class SMapNull : public SMap<val_t> {
  public:
    constexpr SMapNull() {}

    std::optional<val_t> find(sxm::Range<const char>) const noexcept override {
        return std::nullopt;
    };
};

}
