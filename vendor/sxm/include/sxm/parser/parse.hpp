#include "sxm/parser/token.hpp"
#include "sxm/error/error.hpp"
#include <expected>
#include <generator>

#pragma once

namespace sxm {

std::generator<std::expected<sxm::Token, sxm::ParserError> &> parse(const char *src);

}
