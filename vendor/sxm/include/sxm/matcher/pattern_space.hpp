#include "sxm/matcher/pattern.hpp"

#pragma once

namespace {

inline bool char_isspace(char c) {
    return (c == ' ' || c == '\r' || c == '\n' || c == '\t');
}

}

namespace sxm {

template <Pattern_Kind idx_next>

std::optional<Pattern_Match> pattern_space(const char *src, const Pattern_Context *) {
    const char *src_now = src;

    if (char_isspace(*src_now)) {
        ++src_now;

        while (char_isspace(*src_now)) {
            ++src_now;
        }

        return Pattern_Match{
            .token = {
                .kind = Token_Kind::Space,
                .section_start = src,
                .section_end = src_now,
            },

            .idx_next = idx_next,
        };
    }

    return std::nullopt;
};

}
