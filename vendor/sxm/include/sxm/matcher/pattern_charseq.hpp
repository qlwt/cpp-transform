#include "sxm/matcher/pattern.hpp"
#include "sxm/parser/token.hpp"
#include <array>
#include <cstddef>

#pragma once

namespace sxm {

template <std::size_t length, std::array<char, length> expectation, Token_Kind token_kind, Pattern_Kind idx_next>

std::optional<Pattern_Match> pattern_charseq(const char *src, const Pattern_Context *) {
    const char *const src_start = src;
    const char *src_now = src_start;

    for (std::size_t i = 0; i < length; ++i) {
        if (src_now[i] != expectation[i]) {
            return std::nullopt;
        }
    }

    return Pattern_Match{
        .token = {
            .kind = token_kind,
            .section_start = src,
            .section_end = src + length,
        },

        .idx_next = idx_next,
    };
};

}
