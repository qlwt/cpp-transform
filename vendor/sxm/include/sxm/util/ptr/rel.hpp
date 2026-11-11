#pragma once

namespace sxm {

template <typename T, short Dir>

struct PtrRel {
    static_assert(Dir == 1 || Dir == -1, "PtrRel::Dir can be either 1 or -1");

    static inline long size_raw(PtrRel<T, Dir> start, PtrRel<T, Dir> end) noexcept {
        if constexpr (Dir == 1) {
            return end.offset - start.offset;
        } else if constexpr (Dir == -1) {
            return start.offset - end.offset;
        }
    }

    static inline long size_list(PtrRel<T, Dir> start, PtrRel<T, Dir> end) noexcept {
        return PtrRel<T, Dir>::size_raw(start, end) / sizeof(T *);
    }

    long offset;

    inline T *ptr_let(void *memory) const noexcept {
        if constexpr (Dir == 1) {
            return reinterpret_cast<T *>(reinterpret_cast<char *>(memory) + this->offset);
        } else if constexpr (Dir == -1) {
            return reinterpret_cast<T *>(reinterpret_cast<char *>(memory) - this->offset);
        }
    }

    inline void shift_raw(long shift) noexcept {
        this->offset += shift;
    }

    inline void shift_list(long shift) noexcept {
        this->offset += shift * sizeof(T *);
    }
};

}
