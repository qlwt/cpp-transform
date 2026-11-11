#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <optional>
#include "sxm/util/list/range.hpp"
#include "sxm/util/list/sarray.hpp"
#include "sxm/util/search_map/h.hpp"

#pragma once

namespace sxm {

template <typename val_t>

struct HMapStatic_HNode {
    SArray<char> key;
    val_t value;
};

// trivial types only
template <std::size_t cap, typename val_t>

// dumb hashmap
class HMapStatic : public HMap<val_t> {
  private:
    using HNode = HMapStatic_HNode<val_t>;

    inline bool key_eq(Range<const char> l, Range<char> r) const noexcept {
        if (l.size() != r.size()) { return false; }

        return std::memcmp(l.p_start, r.p_start, r.size()) == 0;
    }

    inline void node_init(HNode *node, Range<const char> key, val_t value) noexcept {
        std::size_t key_size = key.size();
        char *key_cpy_start = new char[key_size];

        std::memcpy(key_cpy_start, key.p_start, key_size);

        node->key.data.p_start = key_cpy_start;
        node->key.data.p_end = key_cpy_start + key_size;
        node->value = value;
    }

  public:
    std::array<SArray<HNode>, cap> bucket_list {};

    // very dumb hash function
    constexpr std::size_t hash_let(Range<const char> key) const noexcept override {
        std::uint64_t hash = 0;

        for (const char *key_now = key.p_start; key_now < key.p_end; ++key_now) {
            hash = hash * 31 + *key_now;
        }

        return hash;
    }

    std::optional<val_t> find(Range<const char> key) const noexcept override {
        std::size_t hash = this->hash_let(key);
        const SArray<HNode> *bucket = this->bucket_list.data() + (hash % cap);

        if (bucket->data.p_start == nullptr) {
            return std::nullopt;
        }

        for (HNode *bucket_now = bucket->data.p_start; bucket_now < bucket->data.p_end; ++bucket_now) {
            if (this->key_eq(key, bucket_now->key.data)) {
                return bucket_now->value;
            }
        }

        return std::nullopt;
    }

    void insert(Range<const char> key, val_t value) noexcept override {
        std::size_t hash = this->hash_let(key);
        SArray<HNode> *bucket = this->bucket_list.data() + (hash % cap);

        // insert into empty bucket
        if (bucket->data.p_start == nullptr) {
            bucket->data.p_start = new HNode[1];
            bucket->data.p_end = bucket->data.p_start + 1;

            this->node_init(bucket->data.p_start, key, value);

            return;
        }

        // try to manually find
        for (HNode *bucket_now = bucket->data.p_start; bucket_now < bucket->data.p_end; ++bucket_now) {
            if (this->key_eq(key, bucket_now->key.data)) {
                bucket_now->value = value;

                return;
            }
        }

        // handle insertion into occupied node
        {
            HNode *next_start = new HNode[bucket->data.size() + 1];
            HNode *next_now = next_start;
            HNode *bucket_now = bucket->data.p_start;

            do {
                *(next_now++) = std::move(*(bucket_now++));
            } while (bucket_now < bucket->data.p_end);

            bucket->data.p_start = next_start;
            bucket->data.p_end = next_now + 1;

            this->node_init(next_now, key, value);
        }
    }

    void remove(Range<const char> key) noexcept override {
        std::size_t hash = this->hash_let(key);
        SArray<HNode> *bucket = this->bucket_list.data() + (hash % cap);

        if (bucket->data.p_start == nullptr) {
            return;
        }

        if (bucket->data.size() == 1) {
            if (!this->key_eq(key, bucket->data.p_start->key.data)) {
                return;
            }

            delete[] bucket->data.p_start->key.data.p_start;
            delete[] bucket->data.p_start;

            bucket->data.p_start = nullptr;
            bucket->data.p_end = nullptr;

            return;
        }

        HNode *bucket_search = bucket->data.p_start;

        do {
            if (!this->key_eq(key, bucket_search->key.data)) {
                ++bucket_search;

                continue;
            }

            HNode *next_start = new HNode[bucket->data.size() - 1];
            HNode *next_now = next_start;

            for (HNode *bucket_now = bucket->data.p_start; bucket_now < bucket_search; ++bucket_now) {
                *(next_now++) = *bucket_now;
            }

            for (HNode *bucket_now = bucket_search + 1; bucket_now < bucket->data.p_end; ++bucket_now) {
                *(next_now++) = *bucket_now;
            }

            delete[] bucket_search->key.data.p_start;
            delete[] bucket->data.p_start;

            bucket->data.p_start = next_start;
            bucket->data.p_end = next_now;

            return;
        } while (bucket_search < bucket->data.p_end);
    }
};

}

