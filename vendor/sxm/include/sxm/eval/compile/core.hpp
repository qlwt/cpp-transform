#include "sxm/ast/fnmap.hpp"
#include "sxm/ast/numvalue.hpp"
#include <cstddef>
#include <variant>

#pragma once

namespace sxm::ceval {

using Error = std::variant<ParserError, EvalError>;

struct Offset {
    std::size_t raw { 0 };

    inline Offset() noexcept {}

    inline Offset(std::size_t raw) noexcept
        : raw(raw) { }

    inline Offset(std::size_t raw, bool flag_const) noexcept
        : raw(raw | ((std::size_t)flag_const << 63)) {}

    inline bool flag_const() const noexcept {
        return this->raw >> 63;
    };

    inline bool flag_state() const noexcept {
        return !this->flag_const();
    };

    inline std::size_t get() const noexcept {
        static constexpr std::size_t flag_const = 1lu << 63;
        static constexpr std::size_t nflag_const = ~flag_const;

        return this->raw & nflag_const;
    };

    inline NumValue *resolve(NumValue *const_list, NumValue *state_list) const noexcept {
        if (this->flag_const()) {
            return const_list + this->get();
        }

        return state_list + this->raw;
    }
};

union Instr_Payload {
    std::size_t length;
    Offset offset;
    numfn::FnRaw fn;
    NumValue *value;
};

enum class Instr_Kind {
    Ret,
    Call,
    ArgPush,

    Set,
    Neg,
    Not,

    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Pow,
    Conj,
    Disj,
    Eq,
    UEq,
    GTE,
    LTE,
    LT,
    GT,
    Assign,
};

struct Instr {
    Instr_Kind kind;
    Instr_Payload left;
    Instr_Payload right;
    Instr_Payload output;
};

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
    Offset offset;
    bool flag_persistent;

    NodeNum() = default;
    NodeNum(Node **slot, Node *parent, Offset offset, bool flag_persistent)
        : Node(ASTNode_Kind::Num, slot, parent), offset(offset), flag_persistent(flag_persistent) {};

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
    }
};

}
