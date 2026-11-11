#include <expected>
#include "sxm/ast/fnmap.hpp"
#include "sxm/eval/compile/core.hpp"
#include "sxm/eval/compile/declaration.hpp"

#pragma once

namespace sxm::ceval {

struct Build_Config {
    const sxm::SMap<numfn::Def> &fnmap = numfn::fnmap;
    const sxm::SMap<sxm::NumValue> &varmap_const;
    const sxm::SMap<Offset> &varmap_state;
    std::size_t varmap_state_size;
};

std::expected<Declaration, Error> build(const char *src) noexcept;
std::expected<Declaration, Error> build(const char *src, const Build_Config &config) noexcept;

}
