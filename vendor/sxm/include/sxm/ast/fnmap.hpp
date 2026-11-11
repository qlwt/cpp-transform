#include "sxm/util/list/range.hpp"
#include "sxm/util/search_map/h_const.hpp"
#include "sxm/eval/flat/core.hpp"
#include "sxm/eval/linked/core.hpp"
#include "sxm/ast/numvalue.hpp"
#include <cstddef>

#pragma once

namespace sxm::numfn {

using Validator = bool (*)(std::size_t arg_size) noexcept;
using FnRaw = sxm::NumValue (*)(Range<sxm::NumValue> arg_list) noexcept;
using FnFEval = sxm::NumValue (*)(Range<sxm::feval::NodeNum *> arg_list) noexcept;
using FnLEval = sxm::NumValue (*)(Range<sxm::leval::NodeNum *> arg_list) noexcept;

struct Def {
    FnRaw fn_raw;
    FnFEval fn_feval;
    FnLEval fn_leval;
    Validator validator;
};

sxm::NumValue max_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue max_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue max_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool max_validator(std::size_t arg_size) noexcept;
constexpr Def max_def { max_fn_raw, max_fn_feval, max_fn_leval, max_validator, };

sxm::NumValue min_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue min_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue min_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool min_validator(std::size_t arg_size) noexcept;
constexpr Def min_def { min_fn_raw, min_fn_feval, min_fn_leval, min_validator, };

sxm::NumValue trunc_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue trunc_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue trunc_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool trunc_validator(std::size_t arg_size) noexcept;
constexpr Def trunc_def { trunc_fn_raw, trunc_fn_feval, trunc_fn_leval, trunc_validator, };

sxm::NumValue round_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue round_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue round_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool round_validator(std::size_t arg_size) noexcept;
constexpr Def round_def { round_fn_raw, round_fn_feval, round_fn_leval, round_validator, };

sxm::NumValue ceil_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue ceil_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue ceil_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool ceil_validator(std::size_t arg_size) noexcept;
constexpr Def ceil_def { ceil_fn_raw, ceil_fn_feval, ceil_fn_leval, ceil_validator, };

sxm::NumValue floor_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue floor_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue floor_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool floor_validator(std::size_t arg_size) noexcept;
constexpr Def floor_def { floor_fn_raw, floor_fn_feval, floor_fn_leval, floor_validator, };

sxm::NumValue if_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue if_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue if_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool if_validator(std::size_t arg_size) noexcept;
constexpr Def if_def { if_fn_raw, if_fn_feval, if_fn_leval, if_validator, };

sxm::NumValue ife_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue ife_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue ife_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool ife_validator(std::size_t arg_size) noexcept;
constexpr Def ife_def { ife_fn_raw, ife_fn_feval, ife_fn_leval, ife_validator, };

sxm::NumValue prepare_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue prepare_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue prepare_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool prepare_validator(std::size_t arg_size) noexcept;
constexpr Def prepare_def { prepare_fn_raw, prepare_fn_feval, prepare_fn_leval, prepare_validator, };

sxm::NumValue effect_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue effect_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue effect_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool effect_validator(std::size_t arg_size) noexcept;
constexpr Def effect_def { effect_fn_raw, effect_fn_feval, effect_fn_leval, effect_validator, };

sxm::NumValue sin_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue sin_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue sin_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool sin_validator(std::size_t arg_size) noexcept;
constexpr Def sin_def { sin_fn_raw, sin_fn_feval, sin_fn_leval, sin_validator, };

sxm::NumValue asin_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue asin_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue asin_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool asin_validator(std::size_t arg_size) noexcept;
constexpr Def asin_def { asin_fn_raw, asin_fn_feval, asin_fn_leval, asin_validator, };

sxm::NumValue cos_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue cos_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue cos_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool cos_validator(std::size_t arg_size) noexcept;
constexpr Def cos_def { cos_fn_raw, cos_fn_feval, cos_fn_leval, cos_validator, };

sxm::NumValue acos_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue acos_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue acos_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool acos_validator(std::size_t arg_size) noexcept;
constexpr Def acos_def { acos_fn_raw, acos_fn_feval, acos_fn_leval, acos_validator, };

sxm::NumValue tan_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue tan_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue tan_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool tan_validator(std::size_t arg_size) noexcept;
constexpr Def tan_def { tan_fn_raw, tan_fn_feval, tan_fn_leval, tan_validator, };

sxm::NumValue atan_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue atan_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue atan_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool atan_validator(std::size_t arg_size) noexcept;
constexpr Def atan_def { atan_fn_raw, atan_fn_feval, atan_fn_leval, atan_validator, };

sxm::NumValue ctan_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue ctan_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue ctan_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool ctan_validator(std::size_t arg_size) noexcept;
constexpr Def ctan_def { ctan_fn_raw, ctan_fn_feval, ctan_fn_leval, ctan_validator, };

sxm::NumValue actan_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue actan_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue actan_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool actan_validator(std::size_t arg_size) noexcept;
constexpr Def actan_def { actan_fn_raw, actan_fn_feval, actan_fn_leval, actan_validator, };

sxm::NumValue ln_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue ln_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue ln_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool ln_validator(std::size_t arg_size) noexcept;
constexpr Def ln_def { ln_fn_raw, ln_fn_feval, ln_fn_leval, ln_validator, };

sxm::NumValue log_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue log_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue log_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool log_validator(std::size_t arg_size) noexcept;
constexpr Def log_def { log_fn_raw, log_fn_feval, log_fn_leval, log_validator, };

sxm::NumValue log2_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue log2_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue log2_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool log2_validator(std::size_t arg_size) noexcept;
constexpr Def log2_def { log2_fn_raw, log2_fn_feval, log2_fn_leval, log2_validator, };

sxm::NumValue log10_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue log10_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue log10_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool log10_validator(std::size_t arg_size) noexcept;
constexpr Def log10_def { log10_fn_raw, log10_fn_feval, log10_fn_leval, log10_validator, };

sxm::NumValue b_fn_raw(Range<sxm::NumValue> arg_list) noexcept;
sxm::NumValue b_fn_feval(Range<sxm::feval::NodeNum *> arg_list) noexcept;
sxm::NumValue b_fn_leval(Range<sxm::leval::NodeNum *> arg_list) noexcept;
bool b_validator(std::size_t arg_size) noexcept;
constexpr Def b_def { b_fn_raw, b_fn_feval, b_fn_leval, b_validator, };

constexpr HMapConst<64, 23, Def> fnmap {
    std::array<HMapConst_HNode<Def>, 23> {
        HMapConst_HNode<Def>{ Range<const char>{ "b" }, sxm::numfn::b_def },
        HMapConst_HNode<Def>{ Range<const char>{ "trunc" }, sxm::numfn::trunc_def },
        HMapConst_HNode<Def>{ Range<const char>{ "floor" }, sxm::numfn::floor_def },
        HMapConst_HNode<Def>{ Range<const char>{ "ceil" }, sxm::numfn::ceil_def },
        HMapConst_HNode<Def>{ Range<const char>{ "round" }, sxm::numfn::round_def },

        HMapConst_HNode<Def>{ Range<const char>{ "min" }, sxm::numfn::min_def },
        HMapConst_HNode<Def>{ Range<const char>{ "max" }, sxm::numfn::max_def },

        HMapConst_HNode<Def>{ Range<const char>{ "if" }, sxm::numfn::if_def },
        HMapConst_HNode<Def>{ Range<const char>{ "ife" }, sxm::numfn::ife_def },
        HMapConst_HNode<Def>{ Range<const char>{ "prepare" }, sxm::numfn::prepare_def },
        HMapConst_HNode<Def>{ Range<const char>{ "effect" }, sxm::numfn::effect_def },

        HMapConst_HNode<Def>{ Range<const char>{ "sin" }, sxm::numfn::sin_def },
        HMapConst_HNode<Def>{ Range<const char>{ "asin" }, sxm::numfn::asin_def },
        HMapConst_HNode<Def>{ Range<const char>{ "cos" }, sxm::numfn::cos_def },
        HMapConst_HNode<Def>{ Range<const char>{ "acos" }, sxm::numfn::acos_def },
        HMapConst_HNode<Def>{ Range<const char>{ "tan" }, sxm::numfn::tan_def },
        HMapConst_HNode<Def>{ Range<const char>{ "atan" }, sxm::numfn::atan_def },
        HMapConst_HNode<Def>{ Range<const char>{ "ctan" }, sxm::numfn::ctan_def },
        HMapConst_HNode<Def>{ Range<const char>{ "actan" }, sxm::numfn::actan_def },

        HMapConst_HNode<Def>{ Range<const char>{ "ln" }, sxm::numfn::ln_def },
        HMapConst_HNode<Def>{ Range<const char>{ "log" }, sxm::numfn::log_def },
        HMapConst_HNode<Def>{ Range<const char>{ "log2" }, sxm::numfn::log2_def },
        HMapConst_HNode<Def>{ Range<const char>{ "log10" }, sxm::numfn::log10_def },
    }
};

}
