#include "sxm/ast/numvalue.hpp"
#include "sxm/ast/node.hpp"
#include "sxm/ast/uop.hpp"
#include "sxm/ast/biop.hpp"
#include "sxm/util/list/range.hpp"
#include <vector>

#pragma once

namespace sxm::peval {

struct Node {
    Node **slot;
    Node *parent;
    ASTNode_Kind kind;

    Node() = default;
    Node(Node **slot, Node *parent, ASTNode_Kind kind)
        : slot(slot), parent(parent), kind(kind) {};

    Node(const Node &src) = delete;
    Node& operator=(const Node &src) = delete;

    Node(Node &&src) = default;
    Node& operator=(Node &&src) = default;

    virtual ~Node() = default;
};

struct NodeNum : Node {
    NumValue value;

    NodeNum() = default;
    NodeNum(Node **slot, Node *parent, ASTNode_Kind kind, NumValue value)
        : Node(slot, parent, kind), value(value) {};

    ~NodeNum() noexcept = default;
};

struct NodeRef : Node {
    Range<const char> name;

    NodeRef(Node **slot, Node *parent, ASTNode_Kind kind, Range<const char> name)
        : Node(slot, parent, kind), name(name) {};

    ~NodeRef() noexcept {
        delete[] this->name.p_start;
    }
};

struct NodePriority : Node {
    Node *child;

    NodePriority(Node **slot, Node *parent, ASTNode_Kind kind, Node *child)
        : Node(slot, parent, kind), child(child) {};

    ~NodePriority() noexcept {
        delete this->child;
    };
};

struct NodeBiOp : Node {
    BinaryOp operation;
    Node *left;
    Node *right;

    NodeBiOp() = default;
    NodeBiOp(Node **slot, Node *parent, ASTNode_Kind kind, BinaryOp operation, Node *left, Node *right)
        : Node(slot, parent, kind), operation(operation), left(left), right(right) {};

    ~NodeBiOp() noexcept {
        delete this->left;
        delete this->right;
    }
};

struct NodeUOp : Node {
    UnaryOp operation;
    Node *child;

    NodeUOp() = default;
    NodeUOp(Node **slot, Node *parent, ASTNode_Kind kind, UnaryOp operation, Node *child)
        : Node(slot, parent, kind), operation(operation), child(child) {};

    ~NodeUOp() noexcept {
        delete this->child;
    }
};

struct NodeFn : Node {
    Range<const char> name;
    std::vector<Node *> arg_list;

    NodeFn(Node **slot, Node *parent, ASTNode_Kind kind, Range<const char> name, std::size_t arg_list_size)
        : Node(slot, parent, kind), name(name), arg_list(arg_list_size) {};

    ~NodeFn() noexcept {
        delete[] this->name.p_start;

        for (Node *arg : this->arg_list) {
            delete arg;
        }
    }
};

}
