#include "sxm/util/list/range.hpp"
#include "sxm/ast/numvalue.hpp"
#include "sxm/ast/fnmap.hpp"
#include <cmath>
#include <cstddef>

namespace {

template <typename Arg, auto Member>
inline sxm::NumValue proj(Arg *ptr) {
    if constexpr (Member == nullptr) {
        return *ptr;
    } else {
        return *ptr->*Member;
    }
}

}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue max_proj(sxm::Range<Arg> arg_list) noexcept {
    double max = proj<Arg, Member>(arg_list.p_start).real;

    for (Arg *arg_now = arg_list.p_start + 1; arg_now < arg_list.p_end; ++arg_now) {
        if (proj<Arg, Member>(arg_now).real > max) {
            max = proj<Arg, Member>(arg_now).real;
        }
    }

    return sxm::NumValue { max, };
}

}

sxm::NumValue sxm::numfn::max_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return max_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::max_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return max_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::max_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return max_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::max_validator(std::size_t arg_size) noexcept {
    return arg_size >= 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue min_proj(sxm::Range<Arg> arg_list) noexcept {
    double min = proj<Arg, Member>(arg_list.p_start).real;

    for (Arg *arg_now = arg_list.p_start + 1; arg_now < arg_list.p_end; ++arg_now) {
        if (proj<Arg, Member>(arg_now).real < min) {
            min = proj<Arg, Member>(arg_now).real;
        }
    }

    return sxm::NumValue { min, };
}

}

sxm::NumValue sxm::numfn::min_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return min_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::min_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return min_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::min_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return min_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::min_validator(std::size_t arg_size) noexcept {
    return arg_size >= 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue ceil_proj(sxm::Range<Arg> arg_list) noexcept {
    double real = std::ceil(proj<Arg, Member>(arg_list.p_start).real);

    return sxm::NumValue { real, };
}

}

sxm::NumValue sxm::numfn::ceil_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return ceil_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::ceil_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return ceil_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::ceil_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return ceil_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::ceil_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue round_proj(sxm::Range<Arg> arg_list) noexcept {
    double real = std::round(proj<Arg, Member>(arg_list.p_start).real);

    return sxm::NumValue { real, };
}

}

sxm::NumValue sxm::numfn::round_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return round_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::round_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return round_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::round_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return round_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::round_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue floor_proj(sxm::Range<Arg> arg_list) noexcept {
    double real = std::floor(proj<Arg, Member>(arg_list.p_start).real);

    return sxm::NumValue { real, };
}

}

sxm::NumValue sxm::numfn::floor_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return floor_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::floor_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return floor_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::floor_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return floor_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::floor_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue trunc_proj(sxm::Range<Arg> arg_list) noexcept {
    double real = std::trunc(proj<Arg, Member>(arg_list.p_start).real);

    return sxm::NumValue { real, };
}

}

sxm::NumValue sxm::numfn::trunc_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return trunc_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::trunc_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return trunc_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::trunc_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return trunc_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::trunc_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue if_proj(sxm::Range<Arg> arg_list) noexcept {
    for (Arg *arg_now = arg_list.p_start; arg_now < arg_list.p_end; ++arg_now) {
        sxm::NumValue arg_now_value = proj<Arg, Member>(arg_now);

        if (std::abs(arg_now_value.real) > sxm::NumValue::TOLERANCE_ABS) {
            return arg_now_value;
        }
    }

    return proj<Arg, Member>(arg_list.p_end - 1);
}

}

sxm::NumValue sxm::numfn::if_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return if_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::if_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return if_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::if_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return if_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::if_validator(std::size_t arg_size) noexcept {
    return arg_size >= 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue ife_proj(sxm::Range<Arg> arg_list) noexcept {
    if (std::abs(proj<Arg, Member>(arg_list.p_start).real) > sxm::NumValue::TOLERANCE_ABS) {
        return proj<Arg, Member>(arg_list.p_start + 1);
    }

    return proj<Arg, Member>(arg_list.p_start + 2);
}

}

sxm::NumValue sxm::numfn::ife_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return ife_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::ife_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return ife_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::ife_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return ife_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::ife_validator(std::size_t arg_size) noexcept {
    return arg_size == 3;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue prepare_proj(sxm::Range<Arg> arg_list) noexcept {
    return proj<Arg, Member>(arg_list.p_end - 1);
}

}

sxm::NumValue sxm::numfn::prepare_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return prepare_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::prepare_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return prepare_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::prepare_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return prepare_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::prepare_validator(std::size_t arg_size) noexcept {
    return arg_size >= 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue effect_proj(sxm::Range<Arg> arg_list) noexcept {
    return proj<Arg, Member>(arg_list.p_start);
}

}

sxm::NumValue sxm::numfn::effect_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return effect_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::effect_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return effect_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::effect_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return effect_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::effect_validator(std::size_t arg_size) noexcept {
    return arg_size >= 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue sin_proj(sxm::Range<Arg> arg_list) noexcept {
    return sxm::NumValue {
        std::sin(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::sin_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return sin_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::sin_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return sin_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::sin_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return sin_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::sin_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue asin_proj(sxm::Range<Arg> arg_list) noexcept {
    return sxm::NumValue {
        std::asin(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::asin_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return asin_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::asin_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return asin_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::asin_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return asin_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::asin_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue cos_proj(sxm::Range<Arg> arg_list) noexcept {
    return sxm::NumValue {
        std::cos(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::cos_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return cos_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::cos_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return cos_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
};

sxm::NumValue sxm::numfn::cos_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return cos_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
};

bool sxm::numfn::cos_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue acos_proj(sxm::Range<Arg> arg_list) noexcept {
    return sxm::NumValue {
        std::acos(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::acos_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return acos_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::acos_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return acos_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
};

sxm::NumValue sxm::numfn::acos_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return acos_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
};

bool sxm::numfn::acos_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue tan_proj(sxm::Range<Arg> arg_list) noexcept {
    return sxm::NumValue {
        std::tan(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::tan_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return tan_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::tan_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return tan_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::tan_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return tan_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::tan_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue atan_proj(sxm::Range<Arg> arg_list) noexcept {
    return sxm::NumValue {
        std::atan(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::atan_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return atan_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::atan_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return atan_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::atan_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return atan_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::atan_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue ctan_proj(sxm::Range<Arg> arg_list) noexcept {
    return sxm::NumValue {
        1 / std::tan(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::ctan_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return ctan_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::ctan_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return ctan_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::ctan_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return ctan_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::ctan_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue actan_proj(sxm::Range<Arg> arg_list) noexcept {
    return sxm::NumValue {
        std::numbers::pi / 2 - std::atan(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::actan_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return actan_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::actan_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return actan_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::actan_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return actan_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::actan_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue log_proj(sxm::Range<Arg> arg_list) noexcept {
    return sxm::NumValue {
        (
            std::log(proj<Arg, Member>(arg_list.p_start + 1).real)
            / std::log(proj<Arg, Member>(arg_list.p_start).real)
        ),
    };
}

}

sxm::NumValue sxm::numfn::log_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return log_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::log_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return log_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::log_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return log_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::log_validator(std::size_t arg_size) noexcept {
    return arg_size == 2;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue ln_proj(sxm::Range<Arg> arg_list) noexcept {
    return sxm::NumValue {
        std::log(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::ln_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return ln_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::ln_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return ln_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::ln_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return ln_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::ln_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue log10_proj(sxm::Range<Arg> arg_list) noexcept {
    return sxm::NumValue {
        std::log10(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::log10_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return log10_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::log10_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return log10_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::log10_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return log10_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::log10_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}


namespace {

template <typename Arg, auto Member>
inline sxm::NumValue log2_proj(sxm::Range<Arg> arg_list) noexcept {
    return sxm::NumValue {
        std::log2(proj<Arg, Member>(arg_list.p_start).real),
    };
}

}

sxm::NumValue sxm::numfn::log2_fn_raw(sxm::Range<sxm::NumValue> arg_list) noexcept {
    return log2_proj<sxm::NumValue, nullptr>(arg_list);
}

sxm::NumValue sxm::numfn::log2_fn_feval(sxm::Range<sxm::feval::NodeNum *> arg_list) noexcept {
    return log2_proj<sxm::feval::NodeNum *, &sxm::feval::NodeNum::value>(arg_list);
}

sxm::NumValue sxm::numfn::log2_fn_leval(sxm::Range<sxm::leval::NodeNum *> arg_list) noexcept {
    return log2_proj<sxm::leval::NodeNum *, &sxm::leval::NodeNum::value>(arg_list);
}

bool sxm::numfn::log2_validator(std::size_t arg_size) noexcept {
    return arg_size == 1;
}


sxm::NumValue sxm::numfn::b_fn_raw(sxm::Range<sxm::NumValue>) noexcept {
    return NumValue { 0, };
}

sxm::NumValue sxm::numfn::b_fn_feval(sxm::Range<sxm::feval::NodeNum *>) noexcept {
    return NumValue { 0, };
}

sxm::NumValue sxm::numfn::b_fn_leval(sxm::Range<sxm::leval::NodeNum *>) noexcept {
    return NumValue { 0, };
}

bool sxm::numfn::b_validator(std::size_t) noexcept {
    return true;
}
