#include "sxm/ast/biop.hpp"
#include "sxm/ast/uop.hpp"
#include "sxm/eval/pure/core.hpp"
#include <format>
#include "sxm/eval/pure/ast.hpp"

using namespace sxm;
using namespace peval;

namespace {

bool node_validate_raw(const Node *root, const Node *parent, Node *const *slot) {
    if (root->parent != parent) {
        return false;
    }

    if (root->slot != slot) {
        return false;
    }

    switch (root->kind) {
    case ASTNode_Kind::Ref:
    case ASTNode_Kind::Num: {
        return true;
    }
    case ASTNode_Kind::UOp: {
        const auto lnode = static_cast<const NodeUOp *>(root);

        return node_validate_raw(lnode->child, lnode, &lnode->child);
    }
    case ASTNode_Kind::BiOp: {
        const auto lnode = static_cast<const NodeBiOp *>(root);

        return node_validate_raw(lnode->left, lnode, &lnode->left) && node_validate_raw(lnode->right, lnode, &lnode->right);
    }
    case ASTNode_Kind::Fn: {
        const auto lnode = static_cast<const NodeFn *>(root);

        Node *const *list_now = lnode->arg_list.data();
        Node *const *list_end = list_now + lnode->arg_list.size();

        for (; list_now < list_end; ++list_now) {
            if (!node_validate_raw(*list_now, lnode, list_now)) {
                return false;
            }
        }


        return true;
    }
    case ASTNode_Kind::Priority: {
        const auto lnode = static_cast<const NodePriority *>(root);

        return node_validate_raw(lnode->child, lnode, &lnode->child);
    }
    }

    return false;
}

}

bool sxm::peval::node_validate(Node *node) noexcept {
    return node_validate_raw(node, nullptr, nullptr);
}

namespace {

void node_tostr_raw(const Node *root, unsigned int space, std::string &res) {
    for (unsigned int i = 0 ; i < space; ++i) {
        res += "\t";
    }

    switch (root->kind) {
    case ASTNode_Kind::Ref: {
        const auto lnode = static_cast<const NodeRef *>(root);

        res += std::format("ref \"{}\"", std::string_view { lnode->name.p_start, lnode->name.p_end });

        break;
    }
    case ASTNode_Kind::Num: {
        const auto lnode = static_cast<const NodeNum *>(root);

        res += std::format("num: {}", lnode->value.real);

        break;
    }
    case ASTNode_Kind::UOp: {
        const auto lnode = static_cast<const NodeUOp *>(root);

        res += std::format("uop \"{}\": (\n", uop_tostr(lnode->operation).data());

        node_tostr_raw(lnode->child, space + 1, res);

        res += "\n";

        for (unsigned int i = 0 ; i < space; ++i) {
            res += "\t";
        }

        res += ")";

        break;
    }
    case ASTNode_Kind::BiOp: {
        const auto lnode = static_cast<const NodeBiOp *>(root);

        res += std::format("biop \"{}\": (\n", biop_tostr(lnode->operation).data());

        node_tostr_raw(lnode->left, space + 1, res);

        res += ",\n";

        node_tostr_raw(lnode->right, space + 1, res);

        res += "\n";

        for (unsigned int i = 0 ; i < space; ++i) {
            res += "\t";
        }

        res += ")";

        break;
    }
    case ASTNode_Kind::Fn: {
        const auto lnode = static_cast<const NodeFn *>(root);

        res += std::format("fn({}) \"{}\"\n", lnode->arg_list.size(), std::string_view { lnode->name.p_start, lnode->name.p_end });

        for (Node *arg : lnode->arg_list) {
            node_tostr_raw(arg, space + 1, res);

            res += ",\n";
        }

        for (unsigned int i = 0 ; i < space; ++i) {
            res += "\t";
        }

        res += ")";

        break;
    }
    case ASTNode_Kind::Priority: {
        const auto lnode = static_cast<const NodePriority *>(root);

        res += "priority: (\n";

        node_tostr_raw(lnode->child, space + 1, res);

        res += "\n";

        for (unsigned int i = 0 ; i < space; ++i) {
            res += "\t";
        }

        res += ")";

        break;
    }
    }
}

}

std::string sxm::peval::node_tostr(Node *node) noexcept {
    std::string res {};

    node_tostr_raw(node, 0, res);

    return res;
}
