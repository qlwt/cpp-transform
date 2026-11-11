#include "sxm/util/list/range.hpp"
#include <cstdlib>
#include <cstring>

#pragma once

namespace sxm {

template <typename val_t>

struct SArray {
    Range<val_t> data;

    SArray() noexcept
        : data {} {}

    SArray(std::size_t size) noexcept {
        this->data.p_start = new val_t[size];
        this->data.p_end = this->data.p_start + size;
    }

    // copy
    SArray(const SArray<val_t> &src) noexcept {
        this->data.p_start = new val_t[src.data.size()];
        this->data.p_end = this->data.p_start + src.data.size();

        std::memcpy(this->data.p_start, src.data.p_start, src.data.size() * sizeof(val_t));
    }

    SArray& operator=(const SArray<val_t> &src) noexcept {
        this->data.p_start = new val_t[src.data.size()];
        this->data.p_end = this->data.p_start + src.data.size();

        std::memcpy(this->data.p_start, src.data.p_start, src.data.size() * sizeof(val_t));

        return *this;
    }

    // move
    SArray(SArray<val_t> &&src) noexcept {
        this->data.p_start = src.data.p_start;
        this->data.p_end = src.data.p_end;

        src.data.p_start = nullptr;
        src.data.p_end = nullptr;
    }

    SArray& operator=(SArray<val_t> &&src) noexcept {
        this->data.p_start = src.data.p_start;
        this->data.p_end = src.data.p_end;

        src.data.p_start = nullptr;
        src.data.p_end = nullptr;

        return *this;
    }

    ~SArray() {
        delete[] this->data.p_start;
    }
};

}
