#include "sxm/util/search_map/h.hpp"
#include "sxm/util/search_map/s.hpp"
#include <optional>

#pragma once

namespace sxm {

template <typename val_t>

class HMapFallback : public sxm::HMap<val_t> {
  public:
    const sxm::SMap<val_t> &back;
    sxm::HMap<val_t> &front;

    HMapFallback(const sxm::SMap<val_t> &back, sxm::HMap<val_t> &front)
        : back(back), front(front) {}

    std::size_t hash_let(sxm::Range<const char> key) const noexcept override {
        return this->front.hash_let(key);
    }

    std::optional<val_t> find(sxm::Range<const char> key) const noexcept override {
        std::optional<val_t> front_value = this->front.find(key);

        if (front_value.has_value()) {
            return front_value.value();
        }

        return this->back.find(key);
    }

    void insert(sxm::Range<const char> key, val_t value) noexcept override {
        this->front.insert(key, value);
    }

    void remove(sxm::Range<const char> key) noexcept override {
        this->front.remove(key);
    }
};

}
