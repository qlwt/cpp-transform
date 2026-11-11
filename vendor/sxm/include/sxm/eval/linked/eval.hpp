#include "sxm/ast/fnmap.hpp"
#include "sxm/ast/numvalue.hpp"
#include "sxm/eval/linked/core.hpp"
#include "sxm/util/search_map/h.hpp"
#include <expected>

#pragma once

namespace sxm::leval {

struct EvalMut_Config {
    HMap<NumValue> &varmap;
    const SMap<numfn::Def> &fnmap = numfn::fnmap;
};

struct Eval_Config {
    const SMap<NumValue> &varmap;
    const SMap<numfn::Def> &fnmap = numfn::fnmap;
};

std::expected<NumValue, Error> eval_mut(const char *src, EvalMut_Config config) noexcept;
std::expected<NumValue, Error> eval(const char *src, Eval_Config config) noexcept;
std::expected<NumValue, Error> eval(const char *src) noexcept;

}
