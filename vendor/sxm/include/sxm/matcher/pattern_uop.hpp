#include "sxm/matcher/pattern.hpp"

#pragma once

namespace sxm {

template <sxm::Pattern_Kind idx_next>

std::optional<sxm::Pattern_Match> pattern_uop(const char *src, const Pattern_Context *) {
    switch (*src) {
    case '+':
        return Pattern_Match {
            .token = {
                .kind = Token_Kind::Space,
                .section_start = src,
                .section_end = src + 1,
            },

            .idx_next = idx_next,
        };
    case '!':
    case '-':
        return Pattern_Match {
            .token = {
                .kind = Token_Kind::UnaryOp,
                .section_start = src,
                .section_end = src + 1,
            },

            .idx_next = idx_next,
        };
    }

    return std::nullopt;
}

}
