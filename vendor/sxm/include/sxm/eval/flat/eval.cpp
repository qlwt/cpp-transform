#include "sxm/eval/flat/core.hpp"
#include "sxm/eval/flat/eval.hpp"
#include "sxm/util/list/range.hpp"
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
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <expected>
#include <utility>

using namespace sxm;
using namespace feval;

namespace {

constexpr std::size_t nodespace_num = sizeof(NodeNum);
constexpr std::size_t nodespace_fn = sizeof(NodeFn);
constexpr std::size_t nodespace_ref = sizeof(NodeRef);
constexpr std::size_t nodespace_uop = sizeof(NodeUOp);
constexpr std::size_t nodespace_biop = sizeof(NodeBiOp);
constexpr std::size_t nodespace_priority = sizeof(NodePriority);

template <typename T>
inline T *ptrshift(T *ptr, std::size_t shift) {
    return reinterpret_cast<T *>(reinterpret_cast<char *>(ptr) + shift);
}

struct MemoryArg;
struct MemoryTree;

struct TraverseState {
    Node *parent;
    Node **treeptr;

    MemoryArg &mem_arg;
    MemoryTree &mem_tree;

    HMap<sxm::NumValue> &varmap;
    const SMap<numfn::Def> &fnmap;
};

struct MemoryArg {
    Range<Node *> range;
    Node **br_now;

    static inline MemoryArg init(std::size_t size) noexcept {
        Node **memory_start = reinterpret_cast<Node **>(std::malloc(size * sizeof(Node *)));

        MemoryArg memory {
            { memory_start, memory_start + size },
            memory_start,
        };

        return memory;
    }

    static inline MemoryArg extend(TraverseState &tstate) noexcept {
        MemoryArg src = tstate.mem_arg;

        std::size_t const dest_size_raw = src.size_raw() * 32;
        std::size_t const dest_size_list = src.size_list() * 32;
        Node **const dest_start = reinterpret_cast<Node **>(std::realloc(src.range.p_start, dest_size_raw));
        Node **const dest_br_now = dest_start + (src.br_now - src.range.p_start);

        if (dest_start != src.range.p_start) {
            for (Node **dest_now = dest_start; dest_now < dest_br_now; ++dest_now) {
                (*dest_now)->slot = dest_now;
            }

            if (tstate.treeptr >= src.range.p_start && tstate.treeptr < src.range.p_end) {
                tstate.treeptr = dest_start + (tstate.treeptr - src.range.p_start);
            }
        }

        return {
            .range = { dest_start, dest_start + dest_size_list, },
            .br_now = dest_br_now,
        };
    }

    inline std::size_t size_raw() const noexcept {
        return this->range.size() * sizeof(Node *);
    }

    inline std::size_t size_list() const noexcept {
        return this->range.size();
    }

    inline std::size_t size_free_raw() const noexcept {
        return (this->range.p_end - this->br_now) * sizeof(Node *);
    }

    inline std::size_t size_free_line() const noexcept {
        return this->range.p_end - this->br_now;
    }

    inline void push() noexcept {
        this->br_now += 1;
    }

    inline void pop() noexcept {
        this->br_now -= 1;
    }

    inline void alloc(std::size_t size) noexcept {
        this->br_now += size;
    }

    inline void free(std::size_t size) noexcept {
        this->br_now -= size;
    }
};

struct MemoryTree {
    static inline MemoryTree init(std::size_t size) {
        char *memory_start = reinterpret_cast<char *>(std::malloc(size));

        MemoryTree memory {
            { memory_start, memory_start + size },
            memory_start + sizeof(Node **),
        };

        *memory.root_point() = memory.tree_point_start();

        return memory;
    }

    static MemoryTree memory_extend(TraverseState &tstate) {
        const std::size_t dest_size = tstate.mem_tree.size_raw() * 4;

        char *dest_start = reinterpret_cast<char *>(std::realloc(tstate.mem_tree.range.p_start, dest_size));

        const long shift = dest_start - tstate.mem_tree.range.p_start;

        char *dest_br_now = ptrshift(tstate.mem_tree.br_now, shift);

        if (dest_start != tstate.mem_tree.range.p_start) {
            tstate.parent = ptrshift(tstate.parent, shift);

            Node **root = reinterpret_cast<Node **>(dest_start);

            *root = ptrshift(*root, shift);
            (*root)->slot = root;

            if (!(tstate.treeptr >= tstate.mem_arg.range.p_start && tstate.treeptr < tstate.mem_arg.range.p_end)) {
                tstate.treeptr = ptrshift(tstate.treeptr, shift);
            }

            {
                Node *target = *root;

                do {

                    switch (target->kind) {
                    case ASTNode_Kind::Num: {
                        target = target->parent;

                        break;
                    }
                    case ASTNode_Kind::Ref: {
                        target = target->parent;

                        break;
                    }
                    case ASTNode_Kind::Priority: {
                        auto lnode = static_cast<NodePriority *>(target);

                        if (lnode->compute_off >= 1) {
                            lnode->compute_off = 0;
                            target = target->parent;
                        } else {
                            lnode->child = ptrshift(lnode->child, shift);

                            if (reinterpret_cast<char *>(lnode->child) < dest_br_now) {
                                lnode->child->parent = target;
                                lnode->child->slot = &lnode->child;

                                target = lnode->child;
                                ++lnode->compute_off;
                            } else {
                                lnode->child->slot = &lnode->child;

                                ++lnode->compute_off;
                            }
                        }

                        break;
                    }
                    case ASTNode_Kind::UOp: {
                        auto lnode = static_cast<NodeUOp *>(target);

                        if (lnode->compute_off >= 1) {
                            lnode->compute_off = 0;
                            target = target->parent;
                        } else {
                            lnode->child = ptrshift(lnode->child, shift);

                            if (reinterpret_cast<char *>(lnode->child) < dest_br_now) {
                                lnode->child->parent = target;
                                lnode->child->slot = &lnode->child;

                                target = lnode->child;
                                ++lnode->compute_off;
                            } else {
                                lnode->child->slot = &lnode->child;

                                ++lnode->compute_off;
                            }
                        }

                        break;
                    }
                    case ASTNode_Kind::BiOp: {
                        auto lnode = static_cast<NodeBiOp *>(target);

                        if (lnode->compute_off >= 2) {
                            lnode->compute_off = 0;
                            target = target->parent;
                        } else {
                            Node **operand_slot = lnode->opr_list + lnode->compute_off;
                            Node *operand_ptr = ptrshift(*operand_slot, shift);

                            *operand_slot = operand_ptr;

                            if (reinterpret_cast<char *>(operand_ptr) < dest_br_now) {
                                operand_ptr->parent = target;
                                operand_ptr->slot = operand_slot;

                                target = operand_ptr;
                                ++lnode->compute_off;
                            } else {
                                operand_ptr->slot = operand_slot;

                                ++lnode->compute_off;
                            }
                        }


                        break;
                    }
                    case ASTNode_Kind::Fn: {
                        auto lnode = static_cast<NodeFn *>(target);

                        if (lnode->compute_off >= lnode->arg_list.size_list()) {
                            lnode->compute_off = 0;
                            target = target->parent;
                        } else {
                            Node **arg = lnode->arg_first(tstate.mem_arg.range.p_start) + lnode->compute_off;

                            *arg = ptrshift(*arg, shift);

                            if (reinterpret_cast<char *>(*arg) < dest_br_now) {
                                (*arg)->parent = target;
                                (*arg)->slot = arg;
                                target = *arg;

                                ++lnode->compute_off;
                            } else {
                                (*arg)->slot = arg;
                                ++lnode->compute_off;
                            }
                        }

                        break;
                    }
                    }
                } while (target != nullptr);
            }

            tstate.treeptr = (*tstate.treeptr)->slot;
        }

        return {
            { dest_start, dest_start + dest_size, },
            dest_br_now,
        };
    }

    Range<char> range;
    char *br_now;

    inline std::size_t size_raw() const noexcept {
        return this->range.size();
    }

    inline std::size_t size_free_raw() const noexcept {
        return this->range.p_end - this->br_now;
    }

    inline Node **root_point() const noexcept {
        return reinterpret_cast<Node **>(this->range.p_start);
    }

    inline NodeNum **root_point_num() const noexcept {
        return reinterpret_cast<NodeNum **>(this->range.p_start);
    }

    inline Node *tree_point_start() const noexcept {
        return reinterpret_cast<Node *>(this->root_point() + 1);
    }

    inline Node *tree_point_now() const noexcept {
        return reinterpret_cast<Node *>(this->br_now);
    }

    inline void tree_alloc_raw(long size) noexcept {
        this->br_now += size;
    }

    inline void tree_setptr(char *target) noexcept {
        this->br_now = target;
    }
};

std::expected<Node *, Error> fold_expr(Node *target, Node *limit, TraverseState &tstate) {
    while (true) {
        switch (target->kind) {
        case ASTNode_Kind::Num: {
            if (target->parent == limit) {
                return target;
            } else {
                target = target->parent;
            }

            break;
        }
        case ASTNode_Kind::Ref: {
            const auto lnode = static_cast<NodeRef *>(target);
            const std::optional<NumValue> ref_value = tstate.varmap.find(lnode->name);

            if (!ref_value.has_value()) {
                return std::unexpected {
                    EvalError {
                        EvalError_Kind::VarUndef,
                        "Undefined Variable " + std::string{lnode->name.p_start, lnode->name.p_end}
                    },
                };
            }

            {
                Node **slot = target->slot;
                Node *parent = target->parent;

                new (target) NodeNum {
                    slot,
                    parent,
                    ref_value.value(),
                };
            }

            if (target->parent == limit) {
                return target;
            } else {
                target = target->parent;
            }

            break;
        }
        case ASTNode_Kind::UOp: {
            const auto lnode = static_cast<NodeUOp *>(target);

            if (lnode->child->kind != ASTNode_Kind::Num) {
                target = lnode->child;

                break;
            }

            {
                const auto lchild = static_cast<NodeNum *>(lnode->child);

                Node **slot = target->slot;
                Node *parent = target->parent;

                new (target) NodeNum {
                    slot,
                    parent,
                    uop_apply(lnode->operation, lchild->value),
                };

                if (target->parent == limit) {
                    return target;
                } else {
                    target = target->parent;

                    break;
                }
            }
        }
        case ASTNode_Kind::BiOp: {
            const auto lnode = static_cast<NodeBiOp *>(target);

            if (lnode->operation == BinaryOp::Assign && lnode->left()->kind == ASTNode_Kind::Ref) {
                if (lnode->right()->kind != ASTNode_Kind::Num) {
                    target = lnode->right();

                    break;
                }

                auto lleft = static_cast<NodeRef *>(lnode->left());
                auto lright = static_cast<NodeNum *>(lnode->right());

                tstate.varmap.insert(lleft->name, lright->value);

                new (lleft) NodeNum {
                    target->slot,
                    target->parent,
                    lright->value,
                };

                *(lleft->slot) = lleft;

                if (lleft->parent == limit) {
                    return lleft;
                } else {
                    target = lleft->parent;
                }

                break;
            } else {
                if (lnode->left()->kind != ASTNode_Kind::Num) {
                    target = lnode->left();

                    break;
                }

                if (lnode->right()->kind != ASTNode_Kind::Num) {
                    target = lnode->right();

                    break;
                }

                auto lleft = static_cast<NodeNum *>(lnode->left());
                auto lright = static_cast<NodeNum *>(lnode->right());

                new (lleft) NodeNum {
                    target->slot,
                    target->parent,
                    biop_apply(lnode->operation, lleft->value, lright->value),
                };

                *(lleft->slot) = lleft;

                if (lleft->parent == limit) {
                    return lleft;
                } else {
                    target = lleft->parent;

                    break;
                }
            }
        }
        case ASTNode_Kind::Priority: {
            const auto lnode = static_cast<NodePriority *>(target);

            return lnode->child;
        }
        case ASTNode_Kind::Fn: {
            const auto lnode = static_cast<NodeFn *>(target);

            return *lnode->arg_last(tstate.mem_arg.range.p_start);
        }
        }
    };
}

}

std::expected<NumValue, Error> sxm::feval::eval_mut(const char *src, EvalMut_Config config) noexcept {
    MemoryArg mem_arg = MemoryArg::init(32);
    MemoryTree mem_tree = MemoryTree::init(1 << 10);

    TraverseState tstate {
        .parent = nullptr,
        .treeptr = mem_tree.root_point(),

        .mem_arg = mem_arg,
        .mem_tree = mem_tree,

        .varmap = config.varmap,
        .fnmap = config.fnmap,
    };

    auto token_gen = parse(src);
    auto token_gen_now = token_gen.begin();

    while (true) {
        std::expected<Token, ParserError> &token_now = *token_gen_now;

        if (!token_now.has_value()) {
            std::free(mem_arg.range.p_start);
            std::free(mem_tree.range.p_start);

            return std::unexpected { token_now.error() };
        }

        switch (token_now->kind) {
        case Token_Kind::FnOpen: {
            std::unreachable();
        }
        case Token_Kind::Space: {
            ++token_gen_now;
            break;
        }
        case Token_Kind::End: {
            auto eval_report = fold_expr(*mem_tree.root_point(), nullptr, tstate);

            if (!eval_report.has_value()) {
                std::free(mem_arg.range.p_start);
                std::free(mem_tree.range.p_start);

                return std::unexpected { eval_report.error() };
            }

            NumValue result = (*mem_tree.root_point_num())->value;

            std::free(mem_tree.range.p_start);
            std::free(mem_arg.range.p_start);

            return result;
        }
        case Token_Kind::NumberInt: {
            if (mem_tree.size_free_raw() < nodespace_num) {
                mem_tree = MemoryTree::memory_extend(tstate);
            }

            new (*tstate.treeptr) NodeNum {
                tstate.treeptr,
                tstate.parent,
                NumValue::parse_int(token_now->section_start, token_now->section_end),
            };

            mem_tree.tree_alloc_raw(nodespace_num);
            ++token_gen_now;
            break;
        }
        case Token_Kind::NumberReal: {
            if (mem_tree.size_free_raw() < nodespace_num) {
                mem_tree = MemoryTree::memory_extend(tstate);
            }

            new (*tstate.treeptr) NodeNum {
                tstate.treeptr,
                tstate.parent,
                NumValue::parse_real(token_now->section_start, token_now->section_end),
            };

            mem_tree.tree_alloc_raw(nodespace_num);

            ++token_gen_now;
            break;
        }
        case Token_Kind::Reference: {
            Range<const char> name { token_now->section_start, token_now->section_end, };

            ++token_gen_now;

            if (!token_now.has_value()) {
                std::free(mem_arg.range.p_start);
                std::free(mem_tree.range.p_start);

                return std::unexpected { token_now.error() };
            }

            if (token_now->kind == Token_Kind::FnOpen) {
                ++token_gen_now;

                if (!token_now.has_value()) {
                    return std::unexpected { token_now.error() };
                }

                if (token_now->kind == Token_Kind::FnClose) {
                    if (mem_tree.size_free_raw() < nodespace_num) {
                        mem_tree = MemoryTree::memory_extend(tstate);
                    }

                    std::optional<numfn::Def> lnode_fndef = tstate.fnmap.find(name);

                    if (!lnode_fndef.has_value()) {
                        std::free(mem_arg.range.p_start);
                        std::free(mem_tree.range.p_start);

                        return std::unexpected {
                            EvalError {
                                EvalError_Kind::FnUndef,
                                "Undefined Function " + std::string { name.p_start, name.p_end, }
                            }
                        };
                    }

                    if (!lnode_fndef->validator(0)) {
                        std::free(mem_arg.range.p_start);
                        std::free(mem_tree.range.p_start);

                        return std::unexpected {
                            EvalError {
                                EvalError_Kind::FnEval,
                                "Failed to evaluate function " + std::string { name.p_start, name.p_end, }
                            }
                        };
                    }

                    new (*tstate.treeptr) NodeNum {
                        tstate.treeptr,
                        tstate.parent,
                        lnode_fndef->fn_feval({})
                    };

                    mem_tree.tree_alloc_raw(nodespace_num);

                    ++token_gen_now;
                } else {
                    if (mem_tree.size_free_raw() < nodespace_fn) {
                        mem_tree = MemoryTree::memory_extend(tstate);
                    }

                    if (mem_arg.size_free_line() < 1) {
                        mem_arg = MemoryArg::extend(tstate);
                    }

                    auto lnode = new (*tstate.treeptr) NodeFn {
                        tstate.treeptr,
                        tstate.parent,
                        { name.p_start, name.p_end, },
                        {
                            { (mem_arg.br_now - mem_arg.range.p_start) * (long)sizeof(Node **) },
                            { (mem_arg.br_now - mem_arg.range.p_start + 1) * (long)sizeof(Node **) },
                        },
                    };

                    tstate.parent = lnode;
                    mem_tree.tree_alloc_raw(nodespace_fn);

                    *lnode->arg_first(mem_arg.range.p_start) = mem_tree.tree_point_now();

                    mem_arg.push();
                    tstate.treeptr = lnode->arg_first(mem_arg.range.p_start);
                }
            } else {
                if (mem_tree.size_free_raw() < nodespace_ref) {
                    mem_tree = MemoryTree::memory_extend(tstate);
                }

                new (*tstate.treeptr) NodeRef {
                    tstate.treeptr,
                    tstate.parent,
                    { name.p_start, name.p_end, }
                };

                mem_tree.tree_alloc_raw(nodespace_ref);
            }

            break;
        }
        case Token_Kind::FnComma: {
            if (mem_tree.size_free_raw() < nodespace_num) {
                mem_tree = MemoryTree::memory_extend(tstate);
            }

            if (mem_arg.size_free_line() < 1) {
                mem_arg = MemoryArg::extend(tstate);
            }

            Node *target = (*tstate.treeptr)->parent;

            while (target->kind != ASTNode_Kind::Fn) {
                target = target->parent;
            }

            {
                auto ltarget = static_cast<NodeFn *>(target);

                auto eval_report = fold_expr(*ltarget->arg_last(mem_arg.range.p_start), nullptr, tstate);

                if (!eval_report.has_value()) {
                    std::free(mem_arg.range.p_start);
                    std::free(mem_tree.range.p_start);

                    return std::unexpected { eval_report.error() };
                }

                mem_arg.push();
                mem_tree.tree_setptr(reinterpret_cast<char *>(eval_report.value()) + nodespace_num);

                ltarget->arg_list_push();

                tstate.parent = target;
                tstate.treeptr = ltarget->arg_last(mem_arg.range.p_start);
                *ltarget->arg_last(mem_arg.range.p_start) = mem_tree.tree_point_now();
            }

            ++token_gen_now;
            break;
        }
        case Token_Kind::FnClose: {
            Node *target = (*tstate.treeptr)->parent;

            while (target->kind != ASTNode_Kind::Fn) {
                target = target->parent;
            }

            tstate.treeptr = target->slot;
            tstate.parent = target->parent;

            {
                auto ltarget = static_cast<NodeFn *>(target);
                Range<const char> name = ltarget->name;

                auto eval_report = fold_expr(*ltarget->arg_last(mem_arg.range.p_start), nullptr, tstate);

                if (!eval_report.has_value()) {
                    std::free(mem_arg.range.p_start);
                    std::free(mem_tree.range.p_start);

                    return std::unexpected { eval_report.error() };
                }

                std::optional<numfn::Def> ltarget_fndef = tstate.fnmap.find(ltarget->name);

                if (!ltarget_fndef.has_value()) {
                    std::free(mem_arg.range.p_start);
                    std::free(mem_tree.range.p_start);

                    return std::unexpected {
                        EvalError {
                            EvalError_Kind::FnUndef,
                            "Could not find Function " + std::string { name.p_start, name.p_end }
                        }
                    };
                }

                if (!ltarget_fndef->validator(ltarget->arg_list.size_list())) {
                    std::free(mem_arg.range.p_start);
                    std::free(mem_tree.range.p_start);

                    return std::unexpected {
                        EvalError {
                            EvalError_Kind::FnEval,
                            "Failed to evaluate function " + std::string { name.p_start, name.p_end }
                        }
                    };
                }

                NumValue ltarget_result = ltarget_fndef->fn_feval({
                    reinterpret_cast<NodeNum **>(ltarget->arg_first(mem_arg.range.p_start)),
                    reinterpret_cast<NodeNum **>(ltarget->arg_last(mem_arg.range.p_start)) + 1,
                });

                mem_arg.free(ltarget->arg_list.size_list());

                {
                    Node **slot = ltarget->slot;
                    Node *parent = ltarget->parent;

                    new (ltarget) NodeNum {
                        slot,
                        parent,
                        ltarget_result,
                    };
                }

                mem_tree.tree_setptr(reinterpret_cast<char *>(target) + nodespace_num);
            }

            ++token_gen_now;
            break;
        }
        case Token_Kind::PriorityOpen: {
            if (mem_tree.size_free_raw() < nodespace_priority) {
                mem_tree = MemoryTree::memory_extend(tstate);
            }

            mem_tree.tree_alloc_raw(nodespace_priority);

            auto node_priority = new (*tstate.treeptr) NodePriority {
                tstate.treeptr,
                tstate.parent,
                mem_tree.tree_point_now(),
            };

            {
                tstate.parent = node_priority;
                tstate.treeptr = &node_priority->child;
            }

            ++token_gen_now;
            break;
        }
        case Token_Kind::PriorityClose: {
            Node *target = (*tstate.treeptr)->parent;

            while (target->kind != ASTNode_Kind::Priority) {
                target = target->parent;
            }

            auto lnode = static_cast<NodePriority *>(target);
            auto eval_report = fold_expr(lnode->child, nullptr, tstate);

            if (!eval_report.has_value()) {
                std::free(mem_arg.range.p_start);
                std::free(mem_tree.range.p_start);

                return std::unexpected { eval_report.error() };
            }

            tstate.treeptr = target->slot;
            tstate.parent = target->parent;

            {
                Node **slot = target->slot;
                Node *parent = target->parent;
                NodeNum *child = static_cast<NodeNum *>(lnode->child);

                new (target) NodeNum {
                    slot,
                    parent,
                    child->value
                };
            }

            mem_tree.tree_setptr(reinterpret_cast<char *>(target) + nodespace_num);

            ++token_gen_now;
            break;
        }
        case Token_Kind::UnaryOp: {
            if (mem_tree.size_free_raw() < nodespace_uop) {
                mem_tree = MemoryTree::memory_extend(tstate);
            }

            const UnaryOp uop = uop_let(token_now->section_start);

            auto node_uop = new (*tstate.treeptr) NodeUOp {
                tstate.treeptr,
                tstate.parent,
                uop,
                reinterpret_cast<Node *>(mem_tree.br_now + nodespace_uop),
            };

            {
                tstate.parent = node_uop;
                tstate.treeptr = &node_uop->child;
            }

            mem_tree.tree_alloc_raw(nodespace_uop);

            ++token_gen_now;
            break;
        }
        case Token_Kind::BinaryOp: {
            if (mem_tree.size_free_raw() < nodespace_biop) {
                mem_tree = MemoryTree::memory_extend(tstate);
            }

            const BinaryOp biop = biop_let(token_now->section_start, token_now->section_end);
            const unsigned short biop_priority = biop_priority_let(biop);

            auto target = *tstate.treeptr;
            while (target->parent != nullptr) {
                if (target->parent->kind == ASTNode_Kind::UOp) {
                    target = target->parent;

                    continue;
                } else if (target->parent->kind == ASTNode_Kind::BiOp) {
                    const auto target_parent = static_cast<NodeBiOp *>(target->parent);
                    const unsigned short target_parent_priority = biop_priority_let(target_parent->operation);

                    if (target_parent_priority >= biop_priority) {
                        target = target_parent;

                        continue;
                    } else {
                        break;
                    }
                }

                break;
            }

            if (!(biop == BinaryOp::Assign && target->kind == ASTNode_Kind::Ref)) {
                std::expected<Node *, Error> eval_report;

                eval_report = fold_expr(target, target->parent, tstate);

                if (!eval_report.has_value()) {
                    std::free(mem_arg.range.p_start);
                    std::free(mem_tree.range.p_start);

                    return std::unexpected { eval_report.error() };
                }

                target = eval_report.value();
                mem_tree.tree_setptr(reinterpret_cast<char *>(target) + nodespace_num);
            }

            auto node_biop = new (mem_tree.br_now) NodeBiOp {
                target->slot,
                target->parent,
                biop,
                target,
                reinterpret_cast<Node *>(mem_tree.br_now + nodespace_biop),
            };

            {
                *(node_biop->slot) = node_biop;
                target->parent = node_biop;
                target->slot = node_biop->left_ptr();
            }

            {
                tstate.parent = node_biop;
                tstate.treeptr = node_biop->right_ptr();
            }

            mem_tree.tree_alloc_raw(nodespace_biop);

            ++token_gen_now;
            break;
        }
        }
    }
}

std::expected<NumValue, Error> sxm::feval::eval(const char *src, Eval_Config config) noexcept {
    HMapStatic<255, NumValue> varmap_local {};
    HMapFallback<NumValue> varmap_fb { config.varmap, varmap_local };

    return sxm::feval::eval_mut(src, {
        .varmap = varmap_fb,
        .fnmap = config.fnmap,
    });
}

std::expected<NumValue, Error> sxm::feval::eval(const char *src) noexcept {
    HMapStatic<255, NumValue> varmap_local {};

    return sxm::feval::eval_mut(src, {
        .varmap = varmap_local,
    });
}
