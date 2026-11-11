#pragma once

namespace sxm {

class NumValue {
  public:
    static constexpr double TOLERANCE_ABS = 1e-12;
    static constexpr double TOLERANCE_ABS_NEG = -1e-12;

    double real;

    static NumValue parse_int(const char *name_start, const char *name_end) noexcept;
    static NumValue parse_real(const char *name_start, const char *name_end) noexcept;

    static NumValue neg(const NumValue &child) noexcept;
    static NumValue lneg(const NumValue &left) noexcept;
    static NumValue add(const NumValue &left, const NumValue &right) noexcept;
    static NumValue sub(const NumValue &left, const NumValue &right) noexcept;
    static NumValue mul(const NumValue &left, const NumValue &right) noexcept;
    static NumValue div(const NumValue &left, const NumValue &right) noexcept;
    static NumValue mod(const NumValue &left, const NumValue &right) noexcept;
    static NumValue pow(const NumValue &left, const NumValue &right) noexcept;

    static NumValue gt(const NumValue &left, const NumValue &right) noexcept;
    static NumValue gte(const NumValue &left, const NumValue &right) noexcept;
    static NumValue lt(const NumValue &left, const NumValue &right) noexcept;
    static NumValue lte(const NumValue &left, const NumValue &right) noexcept;

    static NumValue eq(const NumValue &left, const NumValue &right) noexcept;
    static NumValue ueq(const NumValue &left, const NumValue &right) noexcept;

    static NumValue conj(const NumValue &left, const NumValue &right) noexcept;
    static NumValue disj(const NumValue &left, const NumValue &right) noexcept;
};

}
