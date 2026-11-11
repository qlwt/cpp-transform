#include "sxm/eval/compile/build.hpp"
#include "sxm/ast/fnmap.hpp"
#include "sxm/eval/compile/core.hpp"
#include "sxm/util/list/range.hpp"
#include "sxm/util/search_map/h.hpp"
#include "sxm/util/search_map/h_fallback.hpp"
#include "sxm/util/search_map/h_static.hpp"
#include "sxm/util/search_map/s.hpp"
#include "sxm/ast/biop.hpp"
#include "sxm/ast/node.hpp"
#include "sxm/ast/uop.hpp"
#include "sxm/parser/parse.hpp"
#include "sxm/parser/token.hpp"
#include "sxm/util/search_map/s_nullmap.hpp"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <expected>
#include <utility>

using namespace sxm;
using namespace ceval;

namespace {

bool uop_status_cexpr(NodeUOp *target) noexcept {
    return static_cast<NodeNum *>(target->child)->offset.flag_const();
}

bool biop_status_cexpr(NodeBiOp *target) noexcept {
    bool left_cexpr = static_cast<NodeNum *>(target->left)->offset.flag_const();
    bool right_cexpr = static_cast<NodeNum *>(target->right)->offset.flag_const();

    return left_cexpr && right_cexpr;
}

bool fn_status_cexpr(NodeFn *target) noexcept {
    for (Node *arg : target->arg_list) {
        if (!static_cast<NodeNum *>(arg)->offset.flag_const()) {
            return false;
        }
    }

    return true;
}

template <typename T>
struct MicroVector {
    T *start;
    T *now;
    T *end;

    MicroVector(std::size_t size, std::size_t offset) {
        this->start = reinterpret_cast<T *>(std::malloc(size * sizeof(T)));
        this->now = this->start + offset;
        this->end = this->start + size;
    }

    inline std::size_t size() {
        return this->now - this->start;
    }

    inline std::size_t capacity() {
        return this->end - this->start;
    }

    inline void extend() {
        std::size_t next_capacity = (this->capacity() * 2) + (this->capacity() == 0);

        T *next_start = reinterpret_cast<T *>(std::realloc(this->start, next_capacity * sizeof(T)));

        if (next_start != this->start) {
            this->now = next_start + (this->now - this->start);
            this->start = next_start;
        }

        this->end = next_start + next_capacity;
    }

    inline void fit() {
        std::size_t next_capacity = this->size();

        T *next_start = reinterpret_cast<T *>(std::realloc(this->start, next_capacity * sizeof(T)));

        if (next_start != this->start) {
            this->now = next_start + (this->now - this->start);
            this->start = next_start;
        }

        this->end = next_start + next_capacity;
    }

    inline void set(Offset offset, const T &value) {
        *(this->start + offset.get()) = value;
    }

    inline std::size_t push(const T &value) {
        *this->now = value;

        ++this->now;

        if (this->now >= this->end) {
            this->extend();
        }

        return this->now - this->start - 1;
    }

    inline void shrink(std::size_t size) {
        this->now -= size;
    }

    inline void destroy() {
        std::free(this->start);
    }
};

struct ArgTrack {
    std::size_t now { 0 };
    std::size_t max { 0 };

    inline std::size_t push() {
        return this->now++;
    }

    inline void take(std::size_t diff) {
        this->max = std::max(this->now, this->max);
        this->now -= diff;
    }

    inline void take_silent(std::size_t diff) {
        this->now -= diff;
    }
};

struct StateTrack {
    std::size_t now { 0 };
    std::vector<Offset> hole_list {};

    inline Offset alloc() {
        if (this->hole_list.size() >= 1) {
            Offset hole = this->hole_list.back();

            this->hole_list.pop_back();

            return hole;
        }

        return { this->now++, false };
    }

    inline void free(Offset offset) {
        if (offset.flag_state()) {
            this->hole_list.push_back(offset);
        }
    }

    inline void free_num(NodeNum *num) {
        if (num->offset.flag_state() && !num->flag_persistent) {
            this->free(num->offset);
        }
    }
};

struct ConstTrack {
    MicroVector<NumValue> mvector;
    std::vector<Offset> hole_list;

    ConstTrack(std::size_t size, std::size_t offset)
        : mvector(size, offset), hole_list {} {}

    inline NumValue at(Offset offset) {
        return *(this->mvector.start + offset.get());
    }

    inline Offset push(NumValue value) {
        if (this->hole_list.size() >= 1) {
            Offset hole = this->hole_list.back();

            this->hole_list.pop_back();
            this->mvector.set(hole, value);

            return hole;
        }

        return { this->mvector.push(value), true };
    }

    inline void free(Offset offset) {
        if (offset.flag_const()) {
            this->hole_list.push_back(offset);
        }
    }

    inline void free_num(NodeNum *num) {
        if (num->offset.flag_const() && !num->flag_persistent) {
            this->free(num->offset);
        }
    }

    inline void destroy() {
        this->mvector.destroy();
    }
};

struct TraversalState {
    Node *parent;
    Node **treeptr;

    ArgTrack arg_track;

    ConstTrack const_track;

    StateTrack state_track;
    std::size_t state_rsize;

    Offset output;

    MicroVector<Instr> instr_list;

    HMap<Offset> &varmap_local;
    const SMap<NumValue> &varmap_const;
    const SMap<numfn::Def> &fnmap;

    // deletes things that are not cleared on destructor
    inline void destroy() {
        this->const_track.destroy();
        this->instr_list.destroy();

        delete *this->treeptr;
    }
};

inline Instr_Kind instr_kind_let_uop(UnaryOp op) noexcept {
    switch (op) {
    case UnaryOp::Not: {
        return Instr_Kind::Not;
    }
    case UnaryOp::Neg: {
        return Instr_Kind::Neg;
    }
    case UnaryOp::Pos: {
        return Instr_Kind::Set;
    }
    }

    std::unreachable();
}

inline Instr_Kind instr_kind_let_biop(BinaryOp op) noexcept {
    switch (op) {
    case BinaryOp::Add: {
        return Instr_Kind::Add;
    }
    case BinaryOp::Sub: {
        return Instr_Kind::Sub;
    }
    case BinaryOp::Div: {
        return Instr_Kind::Div;
    }
    case BinaryOp::Mul: {
        return Instr_Kind::Mul;
    }
    case BinaryOp::Mod: {
        return Instr_Kind::Mod;
    }
    case BinaryOp::Pow: {
        return Instr_Kind::Pow;
    }
    case BinaryOp::Assign: {
        return Instr_Kind::Assign;
    }
    case BinaryOp::Eq: {
        return Instr_Kind::Eq;
    }
    case BinaryOp::UEq: {
        return Instr_Kind::UEq;
    }
    case BinaryOp::GT: {
        return Instr_Kind::GT;
    }
    case BinaryOp::GTE: {
        return Instr_Kind::GTE;
    }
    case BinaryOp::LT: {
        return Instr_Kind::LT;
    }
    case BinaryOp::LTE: {
        return Instr_Kind::LTE;
    }
    case BinaryOp::Conj: {
        return Instr_Kind::Conj;
    }
    case BinaryOp::Disj: {
        return Instr_Kind::Disj;
    }
    }

    std::unreachable();
}

std::optional<Error> fold_expr(ceval::Node *target, ceval::Node *limit, TraversalState &tstate) noexcept {
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

            std::optional<Offset> ref_offset = tstate.varmap_local.find(lnode->name);

            if (!ref_offset.has_value()) {
                std::optional<NumValue> ref_value = tstate.varmap_const.find(lnode->name);

                if (ref_value.has_value()) {
                    ref_offset = tstate.const_track.push(ref_value.value());
                } else {
                    return EvalError {
                        EvalError_Kind::VarUndef,
                        "Undefined Variable " + std::string{lnode->name.p_start, lnode->name.p_end}
                    };
                }
            }

            Node **slot = target->slot;
            Node *parent = target->parent;
            Offset res_offset = ref_offset.value();

            *slot = new NodeNum {
                slot,
                parent,
                res_offset,
                true
            };

            tstate.output = res_offset;

            delete lnode;
            target = parent;
            break;
        }
        case ASTNode_Kind::UOp: {
            auto lnode = static_cast<NodeUOp *>(target);

            if (lnode->child->kind != ASTNode_Kind::Num) {
                target = lnode->child;

                break;
            }

            Node **slot = lnode->slot;
            Node *parent = lnode->parent;
            NodeNum *child = static_cast<ceval::NodeNum *>(lnode->child);

            if (uop_status_cexpr(lnode)) {
                NumValue *child_value = tstate.const_track.mvector.start + child->offset.get();
                NumValue res_value = uop_apply(lnode->operation, *child_value);

                Offset res_offset = tstate.const_track.push(res_value);

                *slot = new NodeNum {
                    slot,
                    parent,
                    res_offset,
                    false
                };

                tstate.output = res_offset;
            } else {
                tstate.state_track.free_num(child);

                Offset res_offset = tstate.state_track.alloc();

                tstate.instr_list.push({
                    .kind = instr_kind_let_uop(lnode->operation),
                    .left { .offset { child->offset } },
                    .right {},
                    .output { .offset { res_offset } },
                });

                *slot = new NodeNum {
                    slot,
                    parent,
                    res_offset,
                    false
                };

                tstate.output = res_offset;
            }

            delete lnode;
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
                std::optional<Offset> var_offset = tstate.varmap_local.find(lleft->name);

                Node **slot = lnode->slot;
                Node *parent = lnode->parent;

                // if state is one of reserved variables - it must preserve the slot
                if (
                    lright->offset.flag_const()
                    && !(
                        var_offset.has_value()
                        && var_offset->flag_state()
                        && var_offset->get() < tstate.state_rsize
                    )
                ) {
                    if (var_offset.has_value()) {
                        tstate.const_track.free(*var_offset);
                    }

                    tstate.const_track.free_num(lright);
                    Offset ref_offset_next = tstate.const_track.push(tstate.const_track.at(lright->offset));

                    tstate.varmap_local.insert(lleft->name, ref_offset_next);

                    *slot = new NodeNum {
                        slot,
                        parent,
                        ref_offset_next,
                        true,
                    };

                    tstate.output = ref_offset_next;
                } else {
                    tstate.state_track.free_num(lright);

                    if (!var_offset.has_value() || var_offset->flag_const()) {
                        var_offset = tstate.state_track.alloc();

                        tstate.varmap_local.insert(lleft->name, var_offset.value());
                    }

                    tstate.instr_list.push({
                        .kind = Instr_Kind::Assign,
                        .left {},
                        .right { .offset { lright->offset } },
                        .output { .offset { var_offset.value() } },
                    });

                    *slot = new NodeNum {
                        slot,
                        parent,
                        *var_offset,
                        true,
                    };

                    tstate.output = var_offset.value();
                }

                delete lnode;
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

                Node **slot = lnode->slot;
                Node *parent = lnode->parent;
                auto lleft = static_cast<NodeNum *>(lnode->left);
                auto lright = static_cast<NodeNum *>(lnode->right);

                if (biop_status_cexpr(lnode)) {
                    NumValue *lleft_value = tstate.const_track.mvector.start + lleft->offset.get();
                    NumValue *lright_value = tstate.const_track.mvector.start + lright->offset.get();
                    NumValue res_value = biop_apply(lnode->operation, *lleft_value, *lright_value);

                    tstate.const_track.free_num(lleft);
                    tstate.const_track.free_num(lright);

                    Offset res_offset = tstate.const_track.push(res_value);

                    *slot = new NodeNum {
                        slot,
                        parent,
                        res_offset,
                        false
                    };

                    tstate.output = res_offset;
                } else {
                    tstate.state_track.free_num(lleft);
                    tstate.state_track.free_num(lright);

                    Offset res_offset = tstate.state_track.alloc();

                    tstate.instr_list.push({
                        .kind = instr_kind_let_biop(lnode->operation),
                        .left { .offset { lleft->offset } },
                        .right { .offset { lright->offset } },
                        .output { .offset { res_offset } },
                    });

                    *slot = new NodeNum {
                        slot,
                        parent,
                        res_offset,
                        false
                    };

                    tstate.output = res_offset;
                }

                delete lnode;
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
            Offset child_offset = lchild->offset;
            bool child_flag_persistent = lchild->flag_persistent;

            *slot = new NodeNum {
                slot,
                parent,
                child_offset,
                child_flag_persistent
            };

            tstate.output = child_offset;

            delete lnode;
            target = parent;
            break;
        }
        }
    } while (target != limit);

    return std::nullopt;
}

}

std::expected<Declaration, Error> sxm::ceval::build(const char *src, const Build_Config &config) noexcept {
    Node *root;
    HMapStatic<32, Offset> varmap_local {};
    HMapFallback<Offset> varmap_local_fb { config.varmap_state, varmap_local, };

    TraversalState tstate {
        .parent = nullptr,
        .treeptr = &root,

        .arg_track {},

        .const_track { 16, 0 },

        .state_track { .now = config.varmap_state_size },
        .state_rsize = config.varmap_state_size,

        .output {},

        .instr_list { 16, 0 },

        .varmap_local = varmap_local_fb,
        .varmap_const = config.varmap_const,
        .fnmap = config.fnmap,
    };

    auto token_gen = parse(src);
    auto token_it = token_gen.begin();

    while (true) {
        std::expected<Token, ParserError> &token = *token_it;

        if (!token.has_value()) {
            tstate.destroy();

            return std::unexpected { token.error(), };
        }

        switch (token->kind) {
        case Token_Kind::Space:
        case Token_Kind::FnOpen:
            std::unreachable();
        case Token_Kind::End: {
            std::optional<Error> fold_report = fold_expr(root, nullptr, tstate);

            if (fold_report.has_value()) {
                tstate.destroy();

                return std::unexpected { fold_report.value(), };
            }

            tstate.instr_list.push({
                .kind = Instr_Kind::Ret,
                .left { .offset = tstate.output },
                .right {},
                .output {},
            });

            tstate.instr_list.fit();
            tstate.const_track.mvector.fit();

            delete root;

            return Declaration {
                tstate.arg_track.max,
                tstate.state_track.now,
                { tstate.const_track.mvector.start, tstate.const_track.mvector.end },
                { tstate.instr_list.start, tstate.instr_list.end },
            };
        }
        case Token_Kind::NumberInt: {
            NumValue res_value = NumValue::parse_int(token->section_start, token->section_end);
            Offset res_offset = tstate.const_track.push(res_value);

            *tstate.treeptr = new NodeNum {
                tstate.treeptr,
                tstate.parent,
                res_offset,
                false,
            };

            tstate.output = res_offset;

            ++token_it;
            break;
        }
        case Token_Kind::NumberReal: {
            NumValue res_value = NumValue::parse_real(token->section_start, token->section_end);
            Offset res_offset = tstate.const_track.push(res_value);

            *tstate.treeptr = new NodeNum {
                tstate.treeptr,
                tstate.parent,
                res_offset,
                false,
            };

            tstate.output = res_offset;

            ++token_it;
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

            ++token_it;
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
                tstate.destroy();

                return std::unexpected { fold_report.value(), };
            }

            ++token_it;
            break;
        }
        case Token_Kind::UnaryOp: {
            auto lnode = new NodeUOp {
                tstate.treeptr,
                tstate.parent,
                uop_let(token->section_start),
                nullptr,
            };

            *tstate.treeptr = lnode;

            tstate.parent = lnode;
            tstate.treeptr = &lnode->child;

            ++token_it;
            break;
        }
        case Token_Kind::BinaryOp: {
            BinaryOp operation = biop_let(token->section_start, token->section_end);
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
                    tstate.destroy();

                    return std::unexpected { fold_report.value(), };
                }
            }

            ++token_it;
            break;
        }
        case Token_Kind::Reference: {
            Range<const char> name { token->section_start, token->section_end, };

            ++token_it;

            if (!token.has_value()) {
                tstate.destroy();

                return std::unexpected(token.error());
            }

            if (token->kind == Token_Kind::FnOpen) {
                ++token_it;

                if (!token.has_value()) {
                    tstate.destroy();

                    return std::unexpected(token.error());
                }

                if (token->kind == Token_Kind::FnClose) {
                    std::optional<numfn::Def> fn_def = tstate.fnmap.find(name);

                    if (!fn_def.has_value()) {
                        tstate.destroy();

                        return std::unexpected {
                            EvalError {
                                EvalError_Kind::FnUndef,
                                "Undefined Function " + std::string { name.p_start, name.p_end, }
                            }
                        };
                    }

                    if (!fn_def->validator(0)) {
                        tstate.destroy();

                        return std::unexpected {
                            EvalError {
                                EvalError_Kind::FnEval,
                                "Failed to evaluate function " + std::string { name.p_start, name.p_end, }
                            }
                        };
                    }

                    Offset res_offset = tstate.const_track.push(fn_def->fn_raw({}));

                    *tstate.treeptr = new NodeNum {
                        tstate.treeptr,
                        tstate.parent,
                        res_offset,
                        false,
                    };

                    tstate.output = res_offset;

                    ++token_it;
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

                    tstate.arg_track.push();
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
                tstate.destroy();

                return std::unexpected { fold_report.value() };
            }

            {
                auto lchild = static_cast<NodeNum *>(lnode->arg_list.back());

                tstate.instr_list.push({
                    .kind = Instr_Kind::ArgPush,
                    .left { .offset { lchild->offset } },
                    .right { },
                    .output { },
                });

                tstate.arg_track.push();
                tstate.state_track.free_num(lchild);
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

            ++token_it;
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
                tstate.destroy();

                return std::unexpected { fold_report.value() };
            }

            {
                auto lchild = static_cast<NodeNum *>(lnode->arg_list.back());

                tstate.instr_list.push({
                    .kind = Instr_Kind::ArgPush,
                    .left { .offset { lchild->offset } },
                    .right { },
                    .output { },
                });

                tstate.arg_track.push();
                tstate.state_track.free_num(lchild);
            }

            std::optional<numfn::Def> fn_def = tstate.fnmap.find(lnode->name);

            if (!fn_def.has_value()) {
                Range<const char> name = lnode->name;

                tstate.destroy();

                return std::unexpected {
                    EvalError {
                        EvalError_Kind::FnUndef,
                        "Undefined Function " + std::string { name.p_start, name.p_end, }
                    }
                };
            }

            if (!fn_def->validator(lnode->arg_list.size())) {
                Range<const char> name = lnode->name;

                tstate.destroy();

                return std::unexpected {
                    EvalError {
                        EvalError_Kind::FnEval,
                        "Failed to evaluate function " + std::string { name.p_start, name.p_end, }
                    }
                };
            }

            Node **slot = lnode->slot;
            Node *parent = lnode->parent;

            if (fn_status_cexpr(lnode)) {
                tstate.instr_list.shrink(lnode->arg_list.size());
                tstate.arg_track.take_silent(lnode->arg_list.size());

                NumValue *argv_start = new NumValue[lnode->arg_list.size()];
                NumValue *argv_end = argv_start + lnode->arg_list.size();

                for (std::size_t index = 0; index < lnode->arg_list.size(); ++index) {
                    auto arg_num = reinterpret_cast<NodeNum *>(lnode->arg_list[index]);

                    tstate.const_track.free_num(arg_num);
                    *(argv_start + index) = tstate.const_track.at(arg_num->offset);
                }

                NumValue res = fn_def->fn_raw({ argv_start, argv_end });
                Offset res_offset = tstate.const_track.push(res);

                *slot = new NodeNum {
                    slot,
                    parent,
                    res_offset,
                    false
                };

                tstate.output = res_offset;

                delete[] argv_start;
            } else {
                Offset res_offset = tstate.state_track.alloc();

                tstate.instr_list.push({
                    .kind = Instr_Kind::Call,
                    .left { .fn = fn_def.value().fn_raw },
                    .right { .length = lnode->arg_list.size() },
                    .output { .offset { res_offset } },
                });

                *slot = new NodeNum {
                    slot,
                    parent,
                    res_offset,
                    false
                };

                tstate.output = res_offset;
                tstate.arg_track.take(lnode->arg_list.size());
            }

            tstate.treeptr = slot;
            tstate.parent = parent;

            delete lnode;

            ++token_it;
            break;
        }
        }
    }
}

std::expected<Declaration, Error> sxm::ceval::build(const char *src) noexcept {
    return build(src, {
        .varmap_const = SMapNull<NumValue>(),
        .varmap_state = SMapNull<Offset>(),
        .varmap_state_size = 0,
    });
}
