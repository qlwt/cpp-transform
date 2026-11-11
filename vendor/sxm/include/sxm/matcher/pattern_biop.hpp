#include "sxm/matcher/pattern.hpp"

#pragma once

namespace sxm {

std::optional<Pattern_Match> pattern_biop(const char *src, const Pattern_Context *context);

}
