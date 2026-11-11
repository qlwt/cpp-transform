#include "sxm/util/list/range.hpp"
#include <optional>

#pragma once

namespace sxm {

template <typename val_t>

class SMap {
  public:
    virtual std::optional<val_t> find(sxm::Range<const char>) const noexcept = 0;
};

}
