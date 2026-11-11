#include "sxm/eval/pure/core.hpp"
#include <string>

#pragma once

namespace sxm::peval {

bool node_validate(Node *node) noexcept;
std::string node_tostr(Node *node) noexcept;

}
