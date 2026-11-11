#include "sxm/ast/numvalue.hpp"
#include <string>

#pragma once

namespace sxm {

enum class UnaryOp {
    Pos,
    Neg,
    Not,
};

std::string uop_tostr(sxm::UnaryOp op) noexcept;
sxm::UnaryOp uop_let(const char *section_start) noexcept;
sxm::NumValue uop_apply(sxm::UnaryOp op, const sxm::NumValue &target) noexcept;

}
