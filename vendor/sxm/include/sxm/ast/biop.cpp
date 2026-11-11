#include "sxm/ast/biop.hpp"
#include "sxm/ast/numvalue.hpp"
#include <string>
#include <utility>

std::string sxm::biop_tostr(BinaryOp kind) noexcept {
    switch (kind) {
    case BinaryOp::Add: {
        return "+";
    }
    case BinaryOp::Sub: {
        return "-";
    }
    case BinaryOp::Div: {
        return "/";
    }
    case BinaryOp::Mul: {
        return "*";
    }
    case BinaryOp::Mod: {
        return "%";
    }
    case BinaryOp::Pow: {
        return "^";
    }
    case BinaryOp::Assign: {
        return "=";
    }
    case BinaryOp::Conj: {
        return "&&";
    }
    case BinaryOp::Disj: {
        return "||";
    }
    case BinaryOp::Eq: {
        return "==";
    }
    case BinaryOp::UEq: {
        return "!=";
    }
    case BinaryOp::GT: {
        return ">";
    }
    case BinaryOp::GTE: {
        return ">=";
    }
    case BinaryOp::LT: {
        return "<";
    }
    case BinaryOp::LTE: {
        return "<=";
    }
    }

    std::unreachable();
}

sxm::BinaryOp sxm::biop_let(const char *section_start, const char* section_end) noexcept {
    switch (*section_start) {
    case '+':
        return sxm::BinaryOp::Add;
    case '-':
        return sxm::BinaryOp::Sub;
    case '/':
        return sxm::BinaryOp::Div;
    case '*':
        return sxm::BinaryOp::Mul;
    case '^':
        return sxm::BinaryOp::Pow;
    case '%':
        return sxm::BinaryOp::Mod;
    case '&': {
        const char *section_next = section_start + 1;

        if (section_next < section_end && *section_next == '&') {
            return sxm::BinaryOp::Conj;
        }

        break;
    }
    case '|': {
        const char *section_next = section_start + 1;

        if (section_next < section_end && *section_next == '|') {
            return sxm::BinaryOp::Disj;
        }

        break;
    }
    case '>': {
        const char *section_next = section_start + 1;

        if (section_next < section_end && *section_next == '=') {
            return sxm::BinaryOp::GTE;
        }

        return sxm::BinaryOp::GT;
    }
    case '<': {
        const char *section_next = section_start + 1;

        if (section_next < section_end && *section_next == '=') {
            return sxm::BinaryOp::LTE;
        }

        return sxm::BinaryOp::LT;
    }
    case '=': {
        const char *section_next = section_start + 1;

        if (section_next < section_end && *section_next == '=') {
            return sxm::BinaryOp::Eq;
        }

        return sxm::BinaryOp::Assign;
    }
    case '!': {
        const char *section_next = section_start + 1;

        if (section_next < section_end && *section_next == '=') {
            return sxm::BinaryOp::UEq;
        }

        break;
    }
    }

    std::unreachable();
}

unsigned short sxm::biop_priority_let(sxm::BinaryOp biop) noexcept {
    switch (biop) {
    case sxm::BinaryOp::Eq:
    case sxm::BinaryOp::UEq:
    case sxm::BinaryOp::GT:
    case sxm::BinaryOp::GTE:
    case sxm::BinaryOp::LT:
    case sxm::BinaryOp::LTE:
    case sxm::BinaryOp::Conj:
    case sxm::BinaryOp::Disj:
        return 0;
    case sxm::BinaryOp::Assign:
        return 1;
    case sxm::BinaryOp::Add:
    case sxm::BinaryOp::Sub:
        return 2;
    case sxm::BinaryOp::Mul:
    case sxm::BinaryOp::Div:
    case sxm::BinaryOp::Mod:
        return 3;
    case sxm::BinaryOp::Pow:
        return 4;
    }

    std::unreachable();
}

sxm::NumValue sxm::biop_apply(sxm::BinaryOp op, const sxm::NumValue &left, const sxm::NumValue &right) noexcept {
    switch (op) {
    case sxm::BinaryOp::Mul: {
        return sxm::NumValue::mul(left, right);
    }
    case sxm::BinaryOp::Div: {
        return sxm::NumValue::div(left, right);
    }
    case sxm::BinaryOp::Sub: {
        return sxm::NumValue::sub(left, right);
    }
    case sxm::BinaryOp::Add: {
        return sxm::NumValue::add(left, right);
    }
    case sxm::BinaryOp::Mod: {
        return sxm::NumValue::mod(left, right);
    }
    case sxm::BinaryOp::Pow: {
        return sxm::NumValue::pow(left, right);
    }
    case sxm::BinaryOp::Eq: {
        return sxm::NumValue::eq(left, right);
    }
    case sxm::BinaryOp::UEq: {
        return sxm::NumValue::ueq(left, right);
    }
    case sxm::BinaryOp::LT: {
        return sxm::NumValue::lt(left, right);
    }
    case sxm::BinaryOp::LTE: {
        return sxm::NumValue::lte(left, right);
    }
    case sxm::BinaryOp::GT: {
        return sxm::NumValue::gt(left, right);
    }
    case sxm::BinaryOp::GTE: {
        return sxm::NumValue::gte(left, right);
    }
    case sxm::BinaryOp::Conj: {
        return sxm::NumValue::conj(left, right);
    }
    case sxm::BinaryOp::Disj: {
        return sxm::NumValue::disj(left, right);
    }
    case sxm::BinaryOp::Assign: {
        return right;
    }
    }

    std::unreachable();
}
