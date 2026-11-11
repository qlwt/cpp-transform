#include "sxm/eval/linked/core.hpp"
#include "sxm/eval/linked/eval.hpp"
#include "sxm/util/search_map/h.hpp"
#include "sxm/ast/node.hpp"
#include "sxm/ast/biop.hpp"
#include "sxm/ast/uop.hpp"
#include "sxm/ast/fnmap.hpp"
#include "sxm/ast/numvalue.hpp"
#include "sxm/parser/parse.hpp"
#include "sxm/parser/token.hpp"
#include "sxm/util/search_map/h_fallback.hpp"
#include "sxm/util/search_map/h_static.hpp"
#include <expected>

using namespace sxm;
using namespace leval;

namespace {

struct TraverseState {
    Node *parent;
    Node **treeptr;

    HMap<NumValue> &varmap;
    const SMap<numfn::Def> &fnmap;
};

std::optional<Error> fold_expr(Node *target, Node *limit, TraverseState &tstate) noexcept {
    do {
        switch (target->kind) {
        case ASTNode_Kind::Fn: {
            return std::nullopt;
        }
        case ASTNode_Kind::Num: {
            target = target->parent;

            break;
        }
        case ASTNode_Kind::Ref: {
            auto lnode = static_cast<NodeRef *>(target);

            std::optional<NumValue> ref_value = tstate.varmap.find(lnode->name);

            if (!ref_value.has_value()) {
                return EvalError {
                    EvalError_Kind::VarUndef,
                    "Undefined Variable " + std::string{lnode->name.p_start, lnode->name.p_end}
                } ;
            }

            Node **slot = target->slot;
            Node *parent = target->parent;

            delete lnode;

            *slot = new NodeNum {
                slot,
                parent,
                ref_value.value(),
            };

            target = parent;

            break;
        }
        case ASTNode_Kind::UOp: {
            auto lnode = static_cast<NodeUOp *>(target);

            if (lnode->child->kind != ASTNode_Kind::Num) {
                target = lnode->child;

                break;
            }

            Node **slot = target->slot;
            Node *parent = target->parent;
            NodeNum *lchild = static_cast<NodeNum *>(lnode->child);
            NumValue value = uop_apply(lnode->operation, lchild->value);

            delete lnode;

            *slot = new NodeNum {
                slot,
                parent,
                value,
            };


            target = parent;

            break;
        }
        case ASTNode_Kind::BiOp: {
            auto lnode = static_cast<NodeBiOp *>(target);

            if (lnode->operation == BinaryOp::Assign && lnode->left->kind == ASTNode_Kind::Ref) {
                if (lnode->right->kind != ASTNode_Kind::Num) {
                    target = lnode->right;

                    break;
                }

                auto lleft = static_cast<NodeRef *>(lnode->left);
                auto lright = static_cast<NodeNum *>(lnode->right);

                tstate.varmap.insert(lleft->name, lright->value);

                Node **slot = target->slot;
                Node *parent = target->parent;
                NumValue value = lright->value;

                delete target;

                *slot = new NodeNum {
                    slot,
                    parent,
                    value,
                };

                target = parent;
            } else {
                if (lnode->left->kind != ASTNode_Kind::Num) {
                    target = lnode->left;

                    break;
                }

                if (lnode->right->kind != ASTNode_Kind::Num) {
                    target = lnode->right;

                    break;
                }

                Node **slot = target->slot;
                Node *parent = target->parent;

                NodeNum * lleft = static_cast<NodeNum *>(lnode->left);
                NodeNum * lright = static_cast<NodeNum *>(lnode->right);

                NumValue value = biop_apply(lnode->operation, lleft->value, lright->value);

                delete lnode;

                *slot = new NodeNum {
                    slot,
                    parent,
                    value,
                };

                target = parent;
            }

            break;
        }
        case ASTNode_Kind::Priority: {
            auto lnode = static_cast<NodePriority *>(target);

            if (lnode->child->kind != ASTNode_Kind::Num) {
                target = lnode->child;

                break;
            }

            auto lchild = static_cast<NodeNum *>(lnode->child);

            Node **slot = lnode->slot;
            Node *parent = lnode->parent;
            NumValue value = lchild->value;

            delete lnode;

            *slot = new NodeNum {
                slot,
                parent,
                value,
            };

            target = parent;

            break;
        }
        }
    } while (target != limit);

    return std::nullopt;
}

}

std::expected<NumValue, Error> sxm::leval::eval_mut(const char *src, EvalMut_Config config) noexcept {
    Node *root = nullptr;

    TraverseState tstate = {
        .parent = nullptr,
        .treeptr = &root,

        .varmap = config.varmap,
        .fnmap = config.fnmap,
    };

    auto token_gen = parse(src);
    auto token_gen_now = token_gen.begin();

    while (true) {
        std::expected<Token, ParserError> &token_now = *token_gen_now;

        if (!token_now.has_value()) {
            delete root;

            return std::unexpected { token_now.error(), };
        }

        switch (token_now->kind) {
        case Token_Kind::Space:
        case Token_Kind::FnOpen:
            std::unreachable();
        case Token_Kind::End: {
            std::optional<Error> fold_report = fold_expr(root, nullptr, tstate);

            if (fold_report.has_value()) {
                delete root;

                return std::unexpected { fold_report.value(), };
            }

            NumValue result = static_cast<NodeNum *>(root)->value;

            delete root;

            return result;
        }
        case Token_Kind::NumberInt: {
            *tstate.treeptr = new NodeNum {
                tstate.treeptr,
                tstate.parent,
                NumValue::parse_int(token_now->section_start, token_now->section_end),
            };

            ++token_gen_now;
            break;
        }
        case Token_Kind::NumberReal: {
            *tstate.treeptr = new NodeNum {
                tstate.treeptr,
                tstate.parent,
                NumValue::parse_real(token_now->section_start, token_now->section_end),
            };

            ++token_gen_now;
            break;
        }
        case Token_Kind::PriorityOpen: {
            auto lnode = new NodePriority {
                tstate.treeptr,
                tstate.parent,
                nullptr
            };

            *tstate.treeptr = lnode;

            tstate.parent = lnode;
            tstate.treeptr = &lnode->child;

            ++token_gen_now;
            break;
        }
        case Token_Kind::PriorityClose: {
            Node *target = (*tstate.treeptr)->parent;

            while (target->kind != ASTNode_Kind::Priority) {
                target = target->parent;
            }

            tstate.treeptr = target->slot;
            tstate.parent = target->parent;

            auto lnode = static_cast<NodePriority *>(target);

            std::optional<Error> fold_report = fold_expr(lnode->child, target->parent, tstate);

            if (fold_report.has_value()) {
                delete root;

                return std::unexpected { fold_report.value(), };
            }

            ++token_gen_now;
            break;
        }
        case Token_Kind::UnaryOp: {
            auto lnode = new NodeUOp {
                tstate.treeptr,
                tstate.parent,
                uop_let(token_now->section_start),
                nullptr,
            };

            *tstate.treeptr = lnode;

            tstate.parent = lnode;
            tstate.treeptr = &lnode->child;

            ++token_gen_now;
            break;
        }
        case Token_Kind::BinaryOp: {
            BinaryOp operation = biop_let(token_now->section_start, token_now->section_end);
            unsigned short operation_priority = biop_priority_let(operation);

            auto target = *tstate.treeptr;
            while (target->parent != nullptr) {
                if (target->parent->kind == ASTNode_Kind::UOp) {
                    target = target->parent;

                    continue;
                } else if (target->parent->kind == ASTNode_Kind::BiOp) {
                    const auto target_parent = static_cast<NodeBiOp *>(target->parent);
                    const unsigned short target_parent_priority = biop_priority_let(target_parent->operation);

                    if (target_parent_priority >= operation_priority) {
                        target = target_parent;

                        continue;
                    } else {
                        break;
                    }
                }

                break;
            }

            auto lnode = new NodeBiOp {
                target->slot,
                target->parent,
                operation,
                target,
                nullptr,
            };

            *lnode->slot = lnode;
            target->parent = lnode;
            target->slot = &lnode->left;

            tstate.parent = lnode;
            tstate.treeptr = &lnode->right;

            if (!(operation == BinaryOp::Assign && target->kind == ASTNode_Kind::Ref)) {
                std::optional<Error> fold_report = fold_expr(target, lnode, tstate);

                if (fold_report.has_value()) {
                    delete root;

                    return std::unexpected { fold_report.value(), };
                }
            }

            ++token_gen_now;
            break;
        }
        case Token_Kind::Reference: {
            Range<const char> name { token_now->section_start, token_now->section_end, };

            ++token_gen_now;

            if (!token_now.has_value()) {
                delete root;

                return std::unexpected(token_now.error());
            }

            if (token_now->kind == Token_Kind::FnOpen) {
                ++token_gen_now;

                if (!token_now.has_value()) {
                    delete root;

                    return std::unexpected(token_now.error());
                }

                if (token_now->kind == Token_Kind::FnClose) {
                    std::optional<numfn::Def> fn_def = numfn::fnmap.find(name);

                    if (!fn_def.has_value()) {
                        delete root;

                        return std::unexpected {
                            EvalError {
                                EvalError_Kind::FnUndef,
                                "Undefined Function " + std::string { name.p_start, name.p_end, }
                            }
                        };
                    }

                    if (!fn_def->validator(0)) {
                        delete root;

                        return std::unexpected {
                            EvalError {
                                EvalError_Kind::FnEval,
                                "Failed to evaluate function " + std::string { name.p_start, name.p_end, }
                            }
                        };
                    }

                    *tstate.treeptr = new NodeNum {
                        tstate.treeptr,
                        tstate.parent,
                        fn_def->fn_leval({ nullptr, nullptr }),
                    };

                    ++token_gen_now;
                } else {
                    auto lnode = new NodeFn {
                        tstate.treeptr,
                        tstate.parent,
                        { name.p_start, name.p_end, },
                        std::vector<Node *>(1),
                    };

                    *tstate.treeptr = lnode;

                    tstate.treeptr = lnode->arg_list.data();
                    tstate.parent = lnode;
                }

                break;
            }

            {
                *tstate.treeptr = new NodeRef {
                    tstate.treeptr,
                    tstate.parent,
                    name,
                };

                break;
            }
        }
        case Token_Kind::FnComma: {
            Node *target = tstate.parent;

            while (target->kind != ASTNode_Kind::Fn) {
                target = target->parent;
            }

            auto lnode = static_cast<NodeFn *>(target);

            std::optional<Error> fold_report = fold_expr(lnode->arg_list.back(), lnode, tstate);

            if (fold_report.has_value()) {
                delete root;

                return std::unexpected { fold_report.value() };
            }

            {
                Node **arg_data_before = lnode->arg_list.data();

                lnode->arg_list.emplace_back(nullptr);

                Node **arg_data_after = lnode->arg_list.data();

                if (arg_data_before != arg_data_after) {
                    Node **arg_end = arg_data_after + lnode->arg_list.size() - 1;

                    for (Node **arg_now = arg_data_after; arg_now < arg_end; ++arg_now) {
                        (*arg_now)->slot = arg_now;
                    }
                }
            }

            tstate.parent = lnode;
            tstate.treeptr = lnode->arg_list.data() + lnode->arg_list.size() - 1;

            ++token_gen_now;
            break;
        }
        case Token_Kind::FnClose: {
            Node *target = tstate.parent;

            while (target->kind != ASTNode_Kind::Fn) {
                target = target->parent;
            }

            auto lnode = static_cast<NodeFn *>(target);

            std::optional<Error> fold_report = fold_expr(lnode->arg_list.back(), lnode, tstate);

            if (fold_report.has_value()) {
                delete root;

                return std::unexpected { fold_report.value() };
            }

            std::optional<numfn::Def> fn_def = numfn::fnmap.find(lnode->name);

            if (!fn_def.has_value()) {
                Range<const char> name = lnode->name;

                delete root;

                return std::unexpected {
                    EvalError {
                        EvalError_Kind::FnUndef,
                        "Undefined Function " + std::string { name.p_start, name.p_end, }
                    }
                };
            }

            if (!fn_def->validator(lnode->arg_list.size())) {
                Range<const char> name = lnode->name;

                delete root;

                return std::unexpected {
                    EvalError {
                        EvalError_Kind::FnEval,
                        "Failed to evaluate function " + std::string { name.p_start, name.p_end, }
                    }
                };
            }

            Node **slot = lnode->slot;
            Node *parent = lnode->parent;

            NumValue fn_value = fn_def->fn_leval({
                reinterpret_cast<NodeNum **>(lnode->arg_list.data()),
                reinterpret_cast<NodeNum **>(lnode->arg_list.data() + lnode->arg_list.size()),
            });

            delete lnode;

            *slot = new NodeNum {
                slot,
                parent,
                fn_value,
            };

            tstate.treeptr = slot;
            tstate.parent = parent;

            ++token_gen_now;
            break;
        }
        }
    }
}

std::expected<NumValue, Error> sxm::leval::eval(const char *src, Eval_Config config) noexcept {
    HMapStatic<255, NumValue> varmap_local {};
    HMapFallback<NumValue> varmap_fb { config.varmap, varmap_local };

    return sxm::leval::eval_mut(src, {
        .varmap = varmap_fb,
        .fnmap = config.fnmap,
    });
}

std::expected<NumValue, Error> sxm::leval::eval(const char *src) noexcept {
    HMapStatic<255, NumValue> varmap_local {};

    return sxm::leval::eval_mut(src, {
        .varmap = varmap_local,
    });
}
