#include "sxm/ast/numvalue.hpp"
#include "sxm/eval/compile/core.hpp"
#include "sxm/util/list/range.hpp"

#pragma once

namespace sxm::ceval {

struct Unit {
    Range<NumValue> arg_list;
    Range<NumValue> state_list;
    Range<Instr> instr_list;

    Unit() = default;
    Unit(Range<NumValue> arg_list, Range<NumValue> state_list, Range<Instr> inst_list)
        : arg_list(arg_list), state_list(state_list), instr_list(inst_list) {};

    Unit(Unit &src) = delete;
    Unit operator=(Unit &src) = delete;

    Unit(Unit &&src) noexcept {
        this->arg_list = src.arg_list;
        this->instr_list = src.instr_list;
        this->state_list = src.state_list;

        src.state_list.p_start = nullptr;
        src.state_list.p_end = nullptr;

        src.arg_list.p_start = nullptr;
        src.arg_list.p_end = nullptr;

        src.instr_list.p_start = nullptr;
        src.instr_list.p_end = nullptr;
    };

    Unit &operator=(Unit &&src) noexcept {
        this->state_list = src.state_list;
        this->arg_list = src.arg_list;
        this->instr_list = src.instr_list;

        src.state_list.p_start = nullptr;
        src.state_list.p_end = nullptr;

        src.arg_list.p_start = nullptr;
        src.arg_list.p_end = nullptr;

        src.instr_list.p_start = nullptr;
        src.instr_list.p_end = nullptr;

        return *this;
    };

    ~Unit() {
        std::free(this->instr_list.p_start);
    }

    NumValue eval() noexcept;
};

}
