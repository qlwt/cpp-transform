#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <optional>
#include "sxm/util/search_map/s.hpp"
#include "sxm/util/list/range.hpp"

#pragma once

namespace sxm {

template <typename val_t>

struct HMapConst_HNode {
    sxm::Range<const char> key;
    val_t value;
};

template <std::size_t bcap, std::size_t ecap, typename val_t>

class HMapConst : public sxm::SMap<val_t> {
  private:
    using HNode = HMapConst_HNode<val_t>;

    inline constexpr bool key_eq(sxm::Range<const char> l, sxm::Range<const char> r) const {
        if (l.p_end - l.p_start != r.p_end - r.p_start) {
            return false;
        }

        while (r.p_start < r.p_end) {
            if (*(l.p_start++) != *(r.p_start++)) {
                return false;
            }
        }

        return true;
    }

  public:
    std::array<HNode, ecap> el_list {};
    std::array<sxm::Range<HNode>, bcap> bucket_list;

    constexpr HMapConst(std::array<HNode, ecap> in_list) noexcept {
        HMapConst_HNode<val_t> *const in_end = in_list.data() + in_list.size();

        // initialise bucket_list pointer
        // because I for some reason is not allowed to do arithmetics over nullptr
        {
            sxm::Range<HNode> *bucket_now = this->bucket_list.data();
            sxm::Range<HNode> *bucket_end = bucket_now + this->bucket_list.size();

            for (; bucket_now < bucket_end; ++bucket_now) {
                bucket_now->p_start = this->el_list.data();
                bucket_now->p_end = this->el_list.data();
            }
        }

        // preallocate for each bucket
        for (HNode *in_now = in_list.data(); in_now < in_end; ++in_now) {
            const std::size_t hash = hash_let(in_now->key);
            const std::size_t idx = hash % this->bucket_list.size();
            sxm::Range<HNode> *bucket_now = this->bucket_list.data() + idx;

            bucket_now->p_end = bucket_now->p_end + 1;
        }

        // align buckets
        {
            std::size_t acc { 0 };
            sxm::Range<HNode> *bucket_now = this->bucket_list.data();
            sxm::Range<HNode> *bucket_end = bucket_now + this->bucket_list.size();

            for (; bucket_now < bucket_end; ++bucket_now) {
                std::size_t bucket_length = bucket_now->p_end - bucket_now->p_start;

                bucket_now->p_start = this->el_list.data() + acc;
                // end range is an active pointer to which items will be inserted later
                bucket_now->p_end = bucket_now->p_start;

                acc += bucket_length;
            }
        }

        // insert
        for (HMapConst_HNode<val_t> *in_now = in_list.data(); in_now < in_end; ++in_now) {
            const std::size_t hash = hash_let(in_now->key);
            const std::size_t idx = hash % this->bucket_list.size();
            sxm::Range<HNode> *bucket_now = this->bucket_list.data() + idx;

            *bucket_now->p_end = *in_now;

            ++bucket_now->p_end;
        }
    }

    // very dumb hash function
    constexpr std::size_t hash_let(sxm::Range<const char> key) const noexcept {
        std::uint64_t hash = 0;

        for (const char *key_now = key.p_start; key_now < key.p_end; ++key_now) {
            hash = hash * 31 + *key_now;
        }

        return hash;
    }

    constexpr std::optional<val_t> find(sxm::Range<const char> key) const noexcept override {
        std::size_t hash = hash_let(key);
        const sxm::Range<HNode> *bucket = this->bucket_list.data() + (hash % bcap);

        if (bucket->p_start == nullptr) {
            return std::nullopt;
        }

        for (HNode *bucket_now = bucket->p_start; bucket_now < bucket->p_end; ++bucket_now) {
            if (key_eq(key, bucket_now->key)) {
                return bucket_now->value;
            }
        }

        return std::nullopt;
    }

    void inspect(const char name[]) const noexcept {
        std::printf("Inspecting HMapConst \"%s\"\n", name);
        std::printf("Buckets %ld, Elements %ld\n", this->bucket_list.size(), this->el_list.size());

        std::size_t empty_size = 0;

        for (std::size_t i = 0; i < this->bucket_list.size(); ++i) {
            const sxm::Range<HNode> *bucket_now = this->bucket_list.data() + i;

            if (bucket_now->size() >= 1) {
                std::printf("Bucket %ld size: %ld\n", i, bucket_now->p_end - bucket_now->p_start);
            } else {
                ++empty_size;
            }
        }

        std::printf("Amount of Empty Buckets: %ld\n", empty_size);
        std::printf("Finishing Inspection of HMapConst \"%s\"\n", name);
    }
};

}
