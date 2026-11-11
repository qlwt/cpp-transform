#include "sxm/ast/fnmap.hpp"
#include "sxm/eval/pure/core.hpp"
#include "sxm/eval/pure/eval.hpp"
#include "sxm/ast/node.hpp"
#include "sxm/ast/numvalue.hpp"
#include "sxm/util/search_map/h_fallback.hpp"
#include "sxm/util/search_map/h_static.hpp"
#include <cstddef>
#include <expected>
#include <string>
#include <utility>
#include <vector>

using namespace sxm;
using namespace peval;

namespace {

struct StackNode {
    bool check;
    Node *ast;
    NumValue *output;
    NumValue *value_list;

    StackNode(bool check, Node *ast, NumValue *output, NumValue *value_list) noexcept
        : check(check), ast(ast), output(output), value_list(value_list) { }

    StackNode(StackNode &src) noexcept
        : check(src.check), ast(src.ast), output(src.output), value_list(src.value_list) { }

    StackNode(StackNode &&src) noexcept
        : check(src.check), ast(src.ast), output(src.output), value_list(src.value_list) {

        src.value_list = nullptr;
    }

    StackNode& operator=(StackNode &src) noexcept {
        this->check = src.check;
        this->ast = src.ast;
        this->output = src.output;
        this->value_list = src.value_list;

        return *this;
    }

    StackNode& operator=(StackNode &&src) noexcept {
        this->check = src.check;
        this->ast = src.ast;
        this->output = src.output;
        this->value_list = src.value_list;

        src.value_list = nullptr;

        return *this;
    }

    ~StackNode() {
        delete[] this->value_list;
    }
};

inline void stack_handle(std::vector<StackNode> *stack, Node *node, NumValue *output) {
    while (node->kind == ASTNode_Kind::Priority) {
        node = static_cast<NodePriority *>(node)->child;
    }

    switch (node->kind) {
    case ASTNode_Kind::Num: {
        *output = static_cast<NodeNum *>(node)->value;

        break;
    }
    case ASTNode_Kind::Ref: {
        stack->emplace_back(
            true,
            node,
            output,
            nullptr
        );

        break;
    }
    default: {
        stack->emplace_back(
            false,
            node,
            output,
            nullptr
        );

        break;
    }
    }
}

inline void handle_uop(StackNode *snode, NodeUOp *nodeop) {
    switch (nodeop->operation) {
    case UnaryOp::Not: {
        *snode->output = NumValue::lneg(*snode->value_list);

        break;
    }
    case UnaryOp::Neg: {
        *snode->output = NumValue::neg(*snode->value_list);

        break;
    }
    case UnaryOp::Pos: {
        *snode->output = *snode->value_list;

        break;
    }
    }

    return;
}

inline void handle_biop(StackNode *snode, NodeBiOp *nodeop, HMap<NumValue> &varmap) {
    switch (nodeop->operation) {
    case BinaryOp::Mul: {
        *snode->output = NumValue::mul(*snode->value_list, snode->value_list[1]);

        break;
    }
    case BinaryOp::Div: {
        *snode->output = NumValue::div(*snode->value_list, snode->value_list[1]);

        break;
    }
    case BinaryOp::Sub: {
        *snode->output = NumValue::sub(*snode->value_list, snode->value_list[1]);

        break;
    }
    case BinaryOp::Add: {
        *snode->output = NumValue::add(*snode->value_list, snode->value_list[1]);

        break;
    }
    case BinaryOp::Mod: {
        *snode->output = NumValue::mod(*snode->value_list, snode->value_list[1]);

        break;
    }
    case BinaryOp::Pow: {
        *snode->output = NumValue::pow(*snode->value_list, snode->value_list[1]);

        break;
    }
    case BinaryOp::Eq: {
        *snode->output = NumValue::eq(*snode->value_list, snode->value_list[1]);

        break;
    }
    case BinaryOp::UEq: {
        *snode->output = NumValue::ueq(*snode->value_list, snode->value_list[1]);

        break;
    }
    case BinaryOp::LT: {
        *snode->output = NumValue::lt(*snode->value_list, snode->value_list[1]);

        break;
    }
    case BinaryOp::LTE: {
        *snode->output = NumValue::lte(*snode->value_list, snode->value_list[1]);

        break;
    }
    case BinaryOp::GT: {
        *snode->output = NumValue::gt(*snode->value_list, snode->value_list[1]);

        break;
    }
    case BinaryOp::GTE: {
        *snode->output = NumValue::gte(*snode->value_list, snode->value_list[1]);

        break;
    }
    case BinaryOp::Conj: {
        *snode->output = NumValue::conj(*snode->value_list, snode->value_list[1]);

        break;
    }
    case BinaryOp::Disj: {
        *snode->output = NumValue::disj(*snode->value_list, snode->value_list[1]);

        break;
    }
    case BinaryOp::Assign: {
        if (nodeop->left->kind == ASTNode_Kind::Ref) {
            auto nodeop_left = static_cast<NodeRef *>(nodeop->left);

            varmap.insert(nodeop_left->name, snode->value_list[1]);
        }

        *snode->output = snode->value_list[1];

        break;
    }
    }

    return;
}

}

std::expected<NumValue, EvalError> sxm::peval::eval_mut(Node *root, EvalMut_Config config) noexcept {
    NumValue result {};
    std::vector<StackNode> stack {};

    {
        stack_handle(&stack, root, &result);
    }

    while (stack.size() >= 1) {
        StackNode *snode = stack.data() + stack.size() - 1;

        if (snode->check) {
            switch (snode->ast->kind) {
            case ASTNode_Kind::Ref: {
                auto noderef = static_cast<NodeRef *>(snode->ast);
                auto ref_value = config.varmap.find(noderef->name);

                if (ref_value.has_value()) {
                    *snode->output = ref_value.value();
                } else {
                    return std::unexpected {
                        EvalError {
                            EvalError_Kind::VarUndef,
                            "Undefined Variable " + std::string { noderef->name.p_start, noderef->name.p_end }
                        },
                    };
                }

                break;
            }
            case ASTNode_Kind::UOp: {
                handle_uop(snode, static_cast<NodeUOp *>(snode->ast));

                break;
            }
            case ASTNode_Kind::BiOp: {
                handle_biop(snode, static_cast<NodeBiOp *>(snode->ast), config.varmap);

                break;
            }
            case ASTNode_Kind::Fn: {
                auto lnode = static_cast<NodeFn *>(snode->ast);
                auto lnode_fndef = config.fnmap.find(lnode->name);

                if (!lnode_fndef.has_value()) {
                    return std::unexpected {
                        EvalError {
                            EvalError_Kind::FnUndef,
                            "Undefined Function " + std::string { lnode->name.p_start, lnode->name.p_end }
                        },
                    };
                }

                if (!lnode_fndef->validator(lnode->arg_list.size())) {
                    return std::unexpected {
                        EvalError {
                            EvalError_Kind::FnEval,
                            "Function Evaluation Error in Function " + std::string { lnode->name.p_start, lnode->name.p_end }
                        },
                    };
                }

                *snode->output = lnode_fndef.value().fn_raw({
                    snode->value_list,
                    snode->value_list + lnode->arg_list.size()
                });

                break;
            }
            default: {
                std::unreachable();
            }
            }

            stack.pop_back();

            continue;
        }

        snode->check = true;

        switch (snode->ast->kind) {
        case ASTNode_Kind::Num:
        case ASTNode_Kind::Ref:
        case ASTNode_Kind::Priority: {
            std::unreachable();
        }
        case ASTNode_Kind::UOp: {
            auto nodeop = static_cast<NodeUOp *>(snode->ast);

            snode->value_list = new NumValue[1];

            stack_handle(&stack, nodeop->child, snode->value_list);

            break;
        }
        case ASTNode_Kind::BiOp: {
            auto nodeop = static_cast<NodeBiOp *>(snode->ast);

            NumValue *value_list = new NumValue[2];

            snode->value_list = value_list;

            stack_handle(&stack, nodeop->right, value_list + 1);

            if (!(nodeop->operation == BinaryOp::Assign && nodeop->left->kind == ASTNode_Kind::Ref)) {
                stack_handle(&stack, nodeop->left, value_list);
            }

            break;
        }
        case ASTNode_Kind::Fn: {
            auto nodefn = static_cast<NodeFn *>(snode->ast);
            std::size_t arg_list_size = nodefn->arg_list.size();

            if (arg_list_size >= 1) {
                NumValue *value_list = new NumValue[arg_list_size];

                snode->value_list = value_list;

                for (std::size_t i = arg_list_size; (i--) >= 1;) {
                    stack_handle(&stack, nodefn->arg_list[i], value_list + i);
                }
            }

            break;
        }
        }
    }

    return result;
}

std::expected<NumValue, EvalError> sxm::peval::eval(Node *root, Eval_Config config) noexcept {
    HMapStatic<255, NumValue> varmap_local {};
    HMapFallback<NumValue> varmap_fb { config.varmap, varmap_local };

    return sxm::peval::eval_mut(root, {
        .varmap = varmap_fb,
        .fnmap = config.fnmap,
    });
}

std::expected<NumValue, EvalError> sxm::peval::eval(Node *root) noexcept {
    HMapStatic<255, NumValue> varmap_local {};

    return sxm::peval::eval_mut(root, {
        .varmap = varmap_local,
    });
}
