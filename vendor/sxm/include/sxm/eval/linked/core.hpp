#include "sxm/ast/node.hpp"
#include "sxm/ast/biop.hpp"
#include "sxm/ast/uop.hpp"
#include "sxm/ast/numvalue.hpp"
#include "sxm/error/error.hpp"
#include "sxm/util/list/range.hpp"
#include <variant>
#include <vector>

#pragma once

namespace sxm::leval {

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

    virtual ~Node() = default;
};

struct NodeNum : Node {
    NumValue value;

    NodeNum() = default;
    NodeNum(Node **slot, Node *parent, NumValue value)
        : Node(ASTNode_Kind::Num, slot, parent), value(value) {};

    ~NodeNum() = default;
};

struct NodeRef : Node {
    Range<const char> name;

    NodeRef(Node **slot, Node *parent, Range<const char> name)
        : Node(ASTNode_Kind::Ref, slot, parent), name(name) {};

    ~NodeRef() = default;
};

struct NodePriority : Node {
    Node *child;

    NodePriority(Node **slot, Node *parent, Node *child)
        : Node(ASTNode_Kind::Priority, slot, parent), child(child) {};

    ~NodePriority() noexcept {
        delete this->child;
    }
};

struct NodeBiOp : Node {
    BinaryOp operation;
    Node *left;
    Node *right;

    NodeBiOp() = default;
    NodeBiOp(Node **slot, Node *parent, BinaryOp operation, Node *left, Node *right)
        : Node(ASTNode_Kind::BiOp, slot, parent), operation(operation), left(left), right(right) {};

    ~NodeBiOp() noexcept {
        delete this->left;
        delete this->right;
    }
};

struct NodeUOp : Node {
    UnaryOp operation;
    Node *child;

    NodeUOp() = default;
    NodeUOp(Node **slot, Node *parent, UnaryOp operation, Node *child)
        : Node(ASTNode_Kind::UOp, slot, parent), operation(operation), child(child) {};

    ~NodeUOp() noexcept {
        delete this->child;
    }
};

struct NodeFn : Node {
    Range<const char> name;
    std::vector<Node *> arg_list;

    NodeFn(Node **slot, Node *parent, Range<const char> name, std::vector<Node *> arg_list)
        : Node(ASTNode_Kind::Fn, slot, parent), name(name), arg_list(arg_list) {};

    ~NodeFn() noexcept {
        for (auto arg : this->arg_list) {
            delete arg;
        }
    };
};

}
