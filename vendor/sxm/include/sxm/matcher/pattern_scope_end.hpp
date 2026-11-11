#include "sxm/matcher/pattern.hpp"

#pragma once

namespace sxm {

std::optional<Pattern_Match> pattern_scope_end(const char *src, const Pattern_Context *context);

}
