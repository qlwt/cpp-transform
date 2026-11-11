#include "sxm/error/error.hpp"
#include "sxm/eval/pure/core.hpp"
#include <expected>

#pragma once

namespace sxm::peval {

std::expected<Node *, ParserError> build(const char *src) noexcept;

}
