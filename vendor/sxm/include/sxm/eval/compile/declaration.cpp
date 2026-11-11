#include "sxm/ast/numvalue.hpp"
#include "sxm/eval/compile/core.hpp"
#include "sxm/eval/compile/unit.hpp"
#include "sxm/util/list/range.hpp"
#include <cstring>
#include <initializer_list>
#include "sxm/eval/compile/declaration.hpp"

using namespace sxm;
using namespace ceval;

namespace {

inline NumValue l_eval(Range<Instr> instr_list, Range<NumValue> const_list, NumValue *state_list, NumValue *arg_list) {
    for (Instr *instr_now = instr_list.p_start;; ++instr_now) {
        NumValue *output = instr_now->output.offset.resolve(const_list.p_start, state_list);

        switch (instr_now->kind) {
        case Instr_Kind::Ret: {
            NumValue left = *instr_now->left.offset.resolve(const_list.p_start, state_list);

            return left;
        }

        case Instr_Kind::Call: {
            *output = instr_now->left.fn({ arg_list - instr_now->right.length, arg_list });

            arg_list -= instr_now->right.length;

            break;
        }
        case Instr_Kind::ArgPush: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);

            *arg_list = *left;

            ++arg_list;

            break;
        }

        case Instr_Kind::Set: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);

            *output = *left;

            break;
        }
        case Instr_Kind::Neg: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::neg(*left);

            break;
        }
        case Instr_Kind::Not: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::lneg(*left);

            break;
        }

        case Instr_Kind::Conj: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);
            NumValue *right = instr_now->right.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::conj(*left, *right);

            break;
        }
        case Instr_Kind::Disj: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);
            NumValue *right = instr_now->right.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::disj(*left, *right);

            break;
        }
        case Instr_Kind::Add: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);
            NumValue *right = instr_now->right.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::add(*left, *right);

            break;
        }
        case Instr_Kind::Sub: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);
            NumValue *right = instr_now->right.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::sub(*left, *right);

            break;
        }
        case Instr_Kind::Mul: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);
            NumValue *right = instr_now->right.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::mul(*left, *right);

            break;
        }
        case Instr_Kind::Div: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);
            NumValue *right = instr_now->right.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::div(*left, *right);

            break;
        }
        case Instr_Kind::Mod: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);
            NumValue *right = instr_now->right.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::mod(*left, *right);

            break;
        }
        case Instr_Kind::Pow: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);
            NumValue *right = instr_now->right.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::pow(*left, *right);

            break;
        }
        case Instr_Kind::Eq: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);
            NumValue *right = instr_now->right.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::eq(*left, *right);

            break;
        }
        case Instr_Kind::UEq: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);
            NumValue *right = instr_now->right.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::ueq(*left, *right);

            break;
        }
        case Instr_Kind::GTE: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);
            NumValue *right = instr_now->right.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::gte(*left, *right);

            break;
        }
        case Instr_Kind::LTE: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);
            NumValue *right = instr_now->right.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::lte(*left, *right);

            break;
        }
        case Instr_Kind::LT: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);
            NumValue *right = instr_now->right.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::lt(*left, *right);

            break;
        }
        case Instr_Kind::GT: {
            NumValue *left = instr_now->left.offset.resolve(const_list.p_start, state_list);
            NumValue *right = instr_now->right.offset.resolve(const_list.p_start, state_list);

            *output = NumValue::gt(*left, *right);

            break;
        }
        case Instr_Kind::Assign: {
            NumValue *right = instr_now->right.offset.resolve(const_list.p_start, state_list);

            *output = *right;

            break;
        }
        }
    }
}

}

NumValue Declaration::eval(Range<NumValue> varmap_list) const noexcept {
    void *memory = std::malloc((this->state_size + this->arg_size) * sizeof(NumValue));

    NumValue *state_list = reinterpret_cast<NumValue *>(memory);
    NumValue *arg_list = state_list + this->state_size;

    if (varmap_list.p_start != nullptr) {
        std::memcpy(state_list, varmap_list.p_start, varmap_list.size() * sizeof(NumValue));
    }

    NumValue result = l_eval(this->instr_list, this->const_list, state_list, arg_list);

    std::free(memory);

    return result;
}

NumValue Declaration::eval(std::initializer_list<NumValue> varmap_list) const noexcept {
    void *memory = std::malloc((this->state_size + this->arg_size) * sizeof(NumValue));

    NumValue *state_list = reinterpret_cast<NumValue *>(memory);
    NumValue *arg_list = state_list + this->state_size;

    {
        NumValue *state_list_now = state_list;
        auto varmap_now = varmap_list.begin();

        for (; varmap_now != varmap_list.end(); ++varmap_now, ++state_list_now) {
            *state_list_now = *varmap_now;
        }
    }

    NumValue result = l_eval(this->instr_list, this->const_list, state_list, arg_list);

    std::free(memory);

    return result;
}

Unit Declaration::init() const noexcept {
    std::size_t bytesize_inst = this->instr_list.size() * sizeof(Instr);
    std::size_t bytesize_state = (this->arg_size + this->state_size) * sizeof(NumValue);

    void *memory = std::malloc(bytesize_inst + bytesize_state);

    Range<Instr> instr_list {
        reinterpret_cast<Instr *>(memory),
        reinterpret_cast<Instr *>(memory) + this->instr_list.size(),
    };

    Range<NumValue> state_list {
        reinterpret_cast<NumValue *>(instr_list.p_end),
        reinterpret_cast<NumValue *>(instr_list.p_end) + this->state_size,
    };

    Range<NumValue> arg_list {
        state_list.p_end,
        state_list.p_end + this->arg_size,
    };

    for (std::size_t index = 0;; ++index) {
        Instr *src_inst = this->instr_list.p_start + index;
        Instr *dest_inst = instr_list.p_start + index;

        dest_inst->kind = src_inst->kind;

        switch (src_inst->kind) {
        case Instr_Kind::Ret: {
            dest_inst->left.value = src_inst->left.offset.resolve(this->const_list.p_start, state_list.p_start);

            return {
                arg_list,
                state_list,
                instr_list,
            };
        }

        case Instr_Kind::Call: {
            dest_inst->left.fn = src_inst->left.fn;
            dest_inst->right.offset = src_inst->right.offset;
            dest_inst->output.value = src_inst->output.offset.resolve(this->const_list.p_start, state_list.p_start);

            break;
        }
        case Instr_Kind::ArgPush: {
            dest_inst->left.value = src_inst->left.offset.resolve(this->const_list.p_start, state_list.p_start);

            break;
        }

        case Instr_Kind::Set:
        case Instr_Kind::Not:
        case Instr_Kind::Neg: {
            dest_inst->left.value = src_inst->left.offset.resolve(this->const_list.p_start, state_list.p_start);
            dest_inst->output.value = src_inst->output.offset.resolve(this->const_list.p_start, state_list.p_start);

            break;
        }

        case Instr_Kind::Add:
        case Instr_Kind::Sub:
        case Instr_Kind::Mul:
        case Instr_Kind::Div:
        case Instr_Kind::Mod:
        case Instr_Kind::Pow:
        case Instr_Kind::Eq:
        case Instr_Kind::UEq:
        case Instr_Kind::GTE:
        case Instr_Kind::LTE:
        case Instr_Kind::Conj:
        case Instr_Kind::Disj:
        case Instr_Kind::LT:
        case Instr_Kind::GT: {
            dest_inst->left.value = src_inst->left.offset.resolve(this->const_list.p_start, state_list.p_start);
            dest_inst->right.value = src_inst->right.offset.resolve(this->const_list.p_start, state_list.p_start);
            dest_inst->output.value = src_inst->output.offset.resolve(this->const_list.p_start, state_list.p_start);

            break;
        }

        case Instr_Kind::Assign: {
            dest_inst->right.value = src_inst->right.offset.resolve(this->const_list.p_start, state_list.p_start);
            dest_inst->output.value = src_inst->output.offset.resolve(this->const_list.p_start, state_list.p_start);

            break;
        }
        }
    }
}
