#include "sxm/ast/uop.hpp"
#include "sxm/ast/numvalue.hpp"
#include <string>
#include <utility>

std::string sxm::uop_tostr(UnaryOp kind) noexcept {
    switch (kind) {
    case UnaryOp::Pos: {
        return "+";
    }
    case UnaryOp::Neg: {
        return "-";
    }
    case UnaryOp::Not: {
        return "!";
    }
    }

    std::unreachable();
}

sxm::UnaryOp sxm::uop_let(const char *section_start) noexcept {
    switch (*section_start) {
    case '+':
        return sxm::UnaryOp::Pos;
    case '-':
        return sxm::UnaryOp::Neg;
    case '!':
        return sxm::UnaryOp::Not;
    }

    std::unreachable();
}

sxm::NumValue sxm::uop_apply(sxm::UnaryOp op, const sxm::NumValue &target) noexcept {
    switch (op) {
    case sxm::UnaryOp::Pos: {
        return target;
    }
    case sxm::UnaryOp::Neg: {
        return sxm::NumValue::neg(target);
    }
    case sxm::UnaryOp::Not: {
        return sxm::NumValue::lneg(target);
    }
    }

    std::unreachable();
}
