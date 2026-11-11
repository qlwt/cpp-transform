#include <cstddef>
#include <iterator>
#include <string_view>
#include <type_traits>

#pragma once

namespace sxm {

template <typename val_t>

struct Range {
    val_t *p_start;
    val_t *p_end;

    inline std::size_t size() const {
        return this->p_end - this->p_start;
    }

    inline val_t *begin() noexcept {
        return this->p_start;
    }
    inline val_t *end() noexcept {
        return this->p_end;
    }
    inline const val_t *cbegin() const noexcept {
        return this->p_start;
    }
    inline const val_t *cend() const noexcept {
        return this->p_end;
    }

    inline std::reverse_iterator<val_t *> crbegin() noexcept {
        return std::reverse_iterator { this->p_end };
    }
    inline std::reverse_iterator<val_t *> crend() noexcept {
        return std::reverse_iterator { this->p_start };
    }
    inline std::reverse_iterator<const val_t *> crbegin() const noexcept {
        return std::reverse_iterator { this->p_end };
    }
    inline std::reverse_iterator<const val_t *> crend() const noexcept {
        return std::reverse_iterator { this->p_start };
    }

    constexpr Range(): p_start(nullptr), p_end(nullptr) {};
    constexpr Range(val_t *start, val_t *end): p_start(start), p_end(end) {};

    constexpr Range(std::string_view src) requires (std::is_same_v<val_t, char>)
        : p_start(src.data()), p_end(src.data() + src.size()) { };

    constexpr Range(std::string_view src) requires (std::is_same_v<val_t, const char>)
        : p_start(src.data()), p_end(src.data() + src.size()) { };
};

template <typename val_t>

struct RangeRel {
    val_t start;
    val_t end;

    inline std::size_t size_raw() const {
        return val_t::size_raw(this->start, this->end);
    }

    inline std::size_t size_list() const {
        return val_t::size_list(this->start, this->end);
    }

    constexpr RangeRel(): start {}, end {} {};
    constexpr RangeRel(val_t start, val_t end): start(start), end(end) {};
};

}
