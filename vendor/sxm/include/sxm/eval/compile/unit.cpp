#include "sxm/ast/numvalue.hpp"
#include "sxm/eval/compile/core.hpp"
#include "sxm/eval/compile/unit.hpp"
#include "sxm/util/list/range.hpp"
#include "sxm/eval/compile/unit.hpp"

using namespace sxm;
using namespace ceval;

NumValue Unit::eval() noexcept {
    NumValue *arg_list_now = this->arg_list.p_start;

    for (Instr *instr_now = this->instr_list.p_start;; ++instr_now) {
        switch (instr_now->kind) {
        case Instr_Kind::Ret: {
            return *instr_now->left.value;
        }

        case Instr_Kind::Call: {
            *instr_now->output.value = instr_now->left.fn({
                arg_list_now - instr_now->right.length,
                arg_list_now
            });

            arg_list_now -= instr_now->right.length;

            break;
        }
        case Instr_Kind::ArgPush: {
            *arg_list_now = *instr_now->left.value;

            ++arg_list_now;

            break;
        }

        case Instr_Kind::Set: {
            *instr_now->output.value = *instr_now->left.value;

            break;
        }
        case Instr_Kind::Neg: {
            *instr_now->output.value = NumValue::neg(*instr_now->left.value);

            break;
        }
        case Instr_Kind::Not: {
            *instr_now->output.value = NumValue::lneg(*instr_now->left.value);

            break;
        }

        case Instr_Kind::Conj: {
            *instr_now->output.value = NumValue::conj(*instr_now->left.value, *instr_now->right.value);

            break;
        }
        case Instr_Kind::Disj: {
            *instr_now->output.value = NumValue::disj(*instr_now->left.value, *instr_now->right.value);

            break;
        }
        case Instr_Kind::Add: {
            *instr_now->output.value = NumValue::add(*instr_now->left.value, *instr_now->right.value);

            break;
        }
        case Instr_Kind::Sub: {
            *instr_now->output.value = NumValue::sub(*instr_now->left.value, *instr_now->right.value);

            break;
        }
        case Instr_Kind::Mul: {
            *instr_now->output.value = NumValue::mul(*instr_now->left.value, *instr_now->right.value);

            break;
        }
        case Instr_Kind::Div: {
            *instr_now->output.value = NumValue::div(*instr_now->left.value, *instr_now->right.value);

            break;
        }
        case Instr_Kind::Mod: {
            *instr_now->output.value = NumValue::mod(*instr_now->left.value, *instr_now->right.value);

            break;
        }
        case Instr_Kind::Pow: {
            *instr_now->output.value = NumValue::pow(*instr_now->left.value, *instr_now->right.value);

            break;
        }
        case Instr_Kind::Eq: {
            *instr_now->output.value = NumValue::eq(*instr_now->left.value, *instr_now->right.value);

            break;
        }
        case Instr_Kind::UEq: {
            *instr_now->output.value = NumValue::ueq(*instr_now->left.value, *instr_now->right.value);

            break;
        }
        case Instr_Kind::GTE: {
            *instr_now->output.value = NumValue::gte(*instr_now->left.value, *instr_now->right.value);

            break;
        }
        case Instr_Kind::LTE: {
            *instr_now->output.value = NumValue::lte(*instr_now->left.value, *instr_now->right.value);

            break;
        }
        case Instr_Kind::LT: {
            *instr_now->output.value = NumValue::lt(*instr_now->left.value, *instr_now->right.value);

            break;
        }
        case Instr_Kind::GT: {
            *instr_now->output.value = NumValue::gt(*instr_now->left.value, *instr_now->right.value);

            break;
        }
        case Instr_Kind::Assign: {
            *instr_now->output.value = *instr_now->right.value;

            break;
        }
        }
    }
}
