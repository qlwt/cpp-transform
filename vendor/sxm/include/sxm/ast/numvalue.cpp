#include "sxm/ast/numvalue.hpp"
#include <charconv>
#include <cstdlib>
#include <cmath>

using namespace sxm;

namespace {

constexpr NumValue PRIMITIVE_ONE = NumValue(1.0);
constexpr NumValue PRIMITIVE_ZERO = NumValue(0.0);

}

NumValue NumValue::parse_int(const char *src_now, const char* src_end) noexcept {
    double real;

    std::from_chars(src_now, src_end, real);

    return NumValue { real, };
}

NumValue NumValue::parse_real(const char *src_now, const char* src_end) noexcept {
    double real;

    std::from_chars(src_now, src_end, real);

    return NumValue { real, };
}

NumValue NumValue::add(const NumValue &left, const NumValue &right) noexcept {
    return NumValue{ left.real + right.real, };
}

NumValue NumValue::sub(const NumValue &left, const NumValue &right) noexcept {
    return NumValue{ left.real - right.real, };
}

NumValue NumValue::mul(const NumValue &left, const NumValue &right) noexcept {
    return NumValue { left.real * right.real, };
}

NumValue NumValue::mod(const NumValue &left, const NumValue &right) noexcept {
    return NumValue { std::fmod(left.real, right.real), };
}

NumValue NumValue::pow(const NumValue &left, const NumValue &right) noexcept {
    return NumValue { std::pow(left.real, right.real), };
}

NumValue NumValue::div(const NumValue &left, const NumValue &right) noexcept {
    return NumValue { left.real / right.real, };
}

NumValue NumValue::eq(const NumValue &left, const NumValue &right) noexcept {
    bool comp = std::abs(left.real - right.real) <= NumValue::TOLERANCE_ABS;

    if (comp) {
        return PRIMITIVE_ONE;
    }

    return PRIMITIVE_ZERO;
}

NumValue NumValue::ueq(const NumValue &left, const NumValue &right) noexcept {
    bool comp = std::abs(left.real - right.real) > NumValue::TOLERANCE_ABS;

    if (comp) {
        return PRIMITIVE_ONE;
    }

    return PRIMITIVE_ZERO;
}

NumValue NumValue::lt(const NumValue &left, const NumValue &right) noexcept {
    bool comp = (right.real - left.real) > NumValue::TOLERANCE_ABS;

    if (comp) {
        return PRIMITIVE_ONE;
    }

    return PRIMITIVE_ZERO;
}

NumValue NumValue::lte(const NumValue &left, const NumValue &right) noexcept {
    bool comp = (right.real - left.real) >= NumValue::TOLERANCE_ABS_NEG;

    if (comp) {
        return PRIMITIVE_ONE;
    }

    return PRIMITIVE_ZERO;
}

NumValue NumValue::gt(const NumValue &left, const NumValue &right) noexcept {
    bool comp = (left.real - right.real) > NumValue::TOLERANCE_ABS;

    if (comp) {
        return PRIMITIVE_ONE;
    }

    return PRIMITIVE_ZERO;
}

NumValue NumValue::gte(const NumValue &left, const NumValue &right) noexcept {
    bool comp = (left.real - right.real) >= NumValue::TOLERANCE_ABS_NEG;

    if (comp) {
        return PRIMITIVE_ONE;
    }

    return PRIMITIVE_ZERO;
}

NumValue NumValue::conj(const NumValue &left, const NumValue &right) noexcept {
    return {
        static_cast<double>(
            std::abs(left.real) > NumValue::TOLERANCE_ABS
            && std::abs(right.real) > NumValue::TOLERANCE_ABS
        )
    };
}

NumValue NumValue::disj(const NumValue &left, const NumValue &right) noexcept {
    return {
        static_cast<double>(
            std::abs(left.real) > NumValue::TOLERANCE_ABS
            || std::abs(right.real) > NumValue::TOLERANCE_ABS
        )
    };
}

NumValue NumValue::neg(const NumValue &child) noexcept {
    return NumValue { -child.real, };
}

NumValue NumValue::lneg(const NumValue &child) noexcept {
    if (std::abs(child.real) <= NumValue::TOLERANCE_ABS) {
        return PRIMITIVE_ONE;
    }

    return PRIMITIVE_ZERO;
}
