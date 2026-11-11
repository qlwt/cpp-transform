#include "sxm/util/list/range.hpp"
#include <cstddef>
#include <cstdio>
#include <optional>
#include "sxm/util/search_map/s.hpp"

#pragma once

namespace sxm {

template <typename val_t>

class HMap : public SMap<val_t> {
  public:
    virtual std::size_t hash_let(Range<const char>) const noexcept = 0;
    virtual std::optional<val_t> find(Range<const char>) const noexcept = 0;
    virtual void insert(Range<const char>, val_t value) noexcept = 0;
    virtual void remove(Range<const char>) noexcept = 0;
};

}
