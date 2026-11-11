#include "sxm/ast/numvalue.hpp"
#include <string>

#pragma once

namespace sxm {

enum class BinaryOp {
    Add,
    Sub,
    Div,
    Mul,
    Mod,
    Pow,
    Assign,

    Eq,
    UEq,

    GT,
    GTE,

    LT,
    LTE,

    Conj,
    Disj,
};

unsigned short biop_priority_let(sxm::BinaryOp biop) noexcept;
BinaryOp biop_let(const char *name_start, const char* name_end) noexcept;

std::string biop_tostr(sxm::BinaryOp op) noexcept;
sxm::NumValue biop_apply(sxm::BinaryOp op, const sxm::NumValue &left, const sxm::NumValue &right) noexcept;

}
