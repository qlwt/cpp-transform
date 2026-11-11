#include "sxm/matcher/pattern.hpp"
#include <cstdio>

#pragma once

namespace sxm {

template <std::size_t length, std::array<Pattern, length> pattern_list>

std::optional<Pattern_Match> pattern_race(const char *src, const Pattern_Context *context) {
    for (std::size_t i = 0; i < length; ++i) {
        const Pattern pattern = pattern_list.at(i);
        std::optional<Pattern_Match> pattern_match = pattern(src, context);

        if (pattern_match.has_value()) {
            return pattern_match;
        }
    }

    return std::nullopt;
};

}
