#include "sxm/ast/numvalue.hpp"
#include "sxm/eval/compile/core.hpp"
#include "sxm/eval/compile/unit.hpp"
#include "sxm/util/list/range.hpp"
#include <initializer_list>

#pragma once

namespace sxm::ceval {

struct Declaration {
    std::size_t arg_size;
    std::size_t state_size;

    Range<NumValue> const_list;
    Range<Instr> instr_list;

    Declaration() = default;
    Declaration(std::size_t arg_size, std::size_t state_size, Range<NumValue> const_list, Range<Instr> inst_list)
        : arg_size(arg_size), state_size(state_size), const_list(const_list), instr_list(inst_list) {};

    Declaration(Declaration &src) = delete;
    Declaration operator=(Declaration &src) = delete;

    Declaration(Declaration &&src) noexcept {
        this->instr_list = src.instr_list;
        this->const_list = src.const_list;
        this->arg_size = src.arg_size;
        this->state_size = src.state_size;

        src.const_list.p_start = nullptr;
        src.const_list.p_end = nullptr;

        src.instr_list.p_start = nullptr;
        src.instr_list.p_end = nullptr;
    };

    Declaration &operator=(Declaration &&src) noexcept {
        this->const_list = src.const_list;
        this->instr_list = src.instr_list;
        this->arg_size = src.arg_size;
        this->state_size = src.state_size;

        src.const_list.p_start = nullptr;
        src.const_list.p_end = nullptr;
        src.instr_list.p_start = nullptr;
        src.instr_list.p_end = nullptr;

        return *this;
    };

    ~Declaration() {
        std::free(this->const_list.p_start);
        std::free(this->instr_list.p_start);
    }

    Unit init() const noexcept;
    NumValue eval(Range<NumValue> varmap_local) const noexcept;
    NumValue eval(std::initializer_list<NumValue> varmap_local) const noexcept;
};

}
