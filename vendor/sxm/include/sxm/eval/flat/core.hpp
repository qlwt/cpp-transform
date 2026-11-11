#include "sxm/error/error.hpp"
#include "sxm/util/list/range.hpp"
#include "sxm/util/ptr/rel.hpp"
#include "sxm/ast/node.hpp"
#include "sxm/ast/uop.hpp"
#include "sxm/ast/biop.hpp"
#include "sxm/ast/numvalue.hpp"
#include <variant>

#pragma once

namespace sxm::feval {

using Error = std::variant<ParserError, EvalError>;

struct Node {
    ASTNode_Kind kind;
    Node **slot;
    Node *parent;

    Node() = default;
    Node(ASTNode_Kind kind, Node **slot, Node *parent)
        : kind(kind), slot(slot), parent(parent) {};

    Node(const Node &src) = delete;
    Node& operator=(const Node &src) = delete;

    Node(Node &&src) = default;
    Node& operator=(Node &&src) = default;
};

struct NodeNum : Node {
    NumValue value;

    NodeNum() = default;
    NodeNum(Node **slot, Node *parent, NumValue value)
        : Node(ASTNode_Kind::Num, slot, parent), value(value) {};
};

struct NodeRef : Node {
    Range<const char> name;

    NodeRef(Node **slot, Node *parent, Range<const char> name)
        : Node(ASTNode_Kind::Ref, slot, parent), name(name) {};
};

struct NodePriority : Node {
    std::size_t compute_off;
    Node *child;

    NodePriority(Node **slot, Node *parent, Node *child)
        : Node(ASTNode_Kind::Priority, slot, parent), compute_off(0), child(child) {};
};

struct NodeBiOp : Node {
    BinaryOp operation;
    std::size_t compute_off;
    Node *opr_list[2];

    NodeBiOp() = default;
    NodeBiOp(Node **slot, Node *parent, BinaryOp operation, Node *left, Node *right)
        : Node(ASTNode_Kind::BiOp, slot, parent), operation(operation), compute_off(0), opr_list { left, right } {};

    inline Node *left() {
        return this->opr_list[0];
    }

    inline Node **left_ptr() {
        return this->opr_list;
    }

    inline Node *right() {
        return this->opr_list[1];
    }

    inline Node **right_ptr() {
        return this->opr_list + 1;
    }
};

struct NodeUOp : Node {
    std::size_t compute_off;
    UnaryOp operation;
    Node *child;

    NodeUOp() = default;
    NodeUOp(Node **slot, Node *parent, UnaryOp operation, Node *child)
        : Node(ASTNode_Kind::UOp, slot, parent), compute_off(0), operation(operation), child(child) {};
};

struct NodeFn : Node {
    Range<const char> name;
    std::size_t compute_off;
    RangeRel<PtrRel<Node *, 1>> arg_list;

    NodeFn(Node **slot, Node *parent, Range<const char> name, RangeRel<PtrRel<Node *, 1>> arg_list)
        : Node(ASTNode_Kind::Fn, slot, parent), name(name), compute_off(0), arg_list(arg_list) {};

    inline void arg_list_push() {
        this->arg_list.end.shift_list(1);
    }

    inline Node **arg_first(Node **memory_start) {
        return this->arg_list.start.ptr_let(memory_start);
    }

    inline Node **arg_last(Node **memory_start) {
        return this->arg_list.end.ptr_let(memory_start) - 1;
    }
};

}
