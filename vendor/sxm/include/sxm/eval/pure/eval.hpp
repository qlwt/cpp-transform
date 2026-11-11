#include "sxm/ast/fnmap.hpp"
#include "sxm/error/error.hpp"
#include "sxm/eval/pure/core.hpp"
#include "sxm/util/search_map/h.hpp"
#include <expected>

#pragma once

namespace sxm::peval {

struct EvalMut_Config {
    HMap<NumValue> &varmap;
    const SMap<numfn::Def> &fnmap = numfn::fnmap;
};

struct Eval_Config {
    const SMap<NumValue> &varmap;
    const SMap<numfn::Def> &fnmap = numfn::fnmap;
};

std::expected<NumValue, EvalError> eval_mut(Node *root, EvalMut_Config config) noexcept;
std::expected<NumValue, EvalError> eval(Node *root, Eval_Config config) noexcept;
std::expected<NumValue, EvalError> eval(Node *root) noexcept;

}

