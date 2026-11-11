#include "sxm/error/error.hpp"
#include "sxm/eval/pure/core.hpp"
#include "sxm/eval/pure/build.hpp"
#include "sxm/ast/node.hpp"
#include "sxm/ast/numvalue.hpp"
#include "sxm/parser/parse.hpp"
#include <cstddef>
#include <expected>
#include <vector>

using namespace sxm;
using namespace peval;

namespace {

inline Range<const char> name_new(const char *src_now, const char *src_end) {
    std::size_t size = src_end - src_now;
    char *const name_start = new char[size];
    std::memcpy(name_start, src_now, size);

    return { name_start, name_start + size, };
}

}

std::expected<Node *, ParserError> sxm::peval::build(const char *src) noexcept {
    Node *root = nullptr;
    Node *parent = nullptr;
    Node **pointer = &root;

    auto token_gen = parse(src);
    auto token_gen_now = token_gen.begin();

    while (true) {
        std::expected<Token, ParserError> &token_now = *token_gen_now;

        if (!token_now.has_value()) {
            delete root;

            return std::unexpected { token_now.error() };
        }

        switch (token_now->kind) {
        case Token_Kind::Space:
        case Token_Kind::FnOpen: {
            std::unreachable();
        }
        case Token_Kind::End: {
            root->slot = nullptr;

            return root;
        }
        case Token_Kind::Reference: {
            Range<const char> name = name_new(token_now->section_start, token_now->section_end);

            ++token_gen_now;
            std::expected<Token, ParserError> &token_next = *token_gen_now;

            if (token_next->kind == Token_Kind::FnOpen) {
                auto node_fn = new NodeFn {
                    pointer,
                    parent,
                    ASTNode_Kind::Fn,
                    name,
                    1,
                };

                *pointer = node_fn;

                parent = node_fn;
                pointer = &node_fn->arg_list.back();

                ++token_gen_now;
            } else {
                *pointer = new NodeRef {
                    pointer,
                    parent,
                    ASTNode_Kind::Ref,
                    name
                };
            }

            break;
        }
        case Token_Kind::NumberInt: {
            *pointer = new NodeNum {
                pointer,
                parent,
                ASTNode_Kind::Num,
                NumValue::parse_int(token_now->section_start, token_now->section_end),
            };

            ++token_gen_now;

            break;
        }
        case Token_Kind::NumberReal: {
            *pointer = new NodeNum {
                pointer,
                parent,
                ASTNode_Kind::Num,
                NumValue::parse_real(token_now->section_start, token_now->section_end),
            };

            ++token_gen_now;

            break;
        }
        case Token_Kind::PriorityOpen: {
            auto node_priority = new NodePriority {
                pointer,
                parent,
                ASTNode_Kind::Priority,
                nullptr
            };

            {
                *pointer = node_priority;
            }

            {
                parent = node_priority;
                pointer = &node_priority->child;
            }

            ++token_gen_now;

            break;
        }
        case Token_Kind::PriorityClose: {
            auto search_ptr = *pointer;

            while (true) {
                if (search_ptr->kind == ASTNode_Kind::Priority) {
                    pointer = search_ptr->slot;
                    parent = search_ptr->parent;

                    break;
                }

                search_ptr = search_ptr->parent;
            }

            ++token_gen_now;

            break;
        }
        case Token_Kind::FnClose: {
            auto search_ptr = parent;

            while (true) {
                if (search_ptr->kind == ASTNode_Kind::Fn) {
                    auto node_fn = static_cast<NodeFn *>(search_ptr);

                    pointer = node_fn->slot;
                    parent = node_fn->parent;

                    if (node_fn->arg_list.back() == nullptr) {
                        node_fn->arg_list.pop_back();
                    }

                    break;
                }

                search_ptr = search_ptr->parent;
            }

            ++token_gen_now;

            break;
        }
        case Token_Kind::FnComma: {
            auto search_ptr = (*pointer)->parent;

            while (true) {
                if (search_ptr->kind == ASTNode_Kind::Fn) {
                    auto node_fn = static_cast<NodeFn *>(search_ptr);

                    {
                        Node **data_before = node_fn->arg_list.data();

                        node_fn->arg_list.emplace_back();

                        Node **data_now = node_fn->arg_list.data();

                        if (data_before != data_now) {
                            // last element is uninitialised
                            Node **data_end = data_now + node_fn->arg_list.size() - 1;

                            for (; data_now < data_end; ++data_now) {
                                (*data_now)->slot = data_now;
                            }
                        }
                    }

                    pointer = &node_fn->arg_list.back();
                    parent = node_fn;

                    break;
                }

                search_ptr = search_ptr->parent;
            }

            ++token_gen_now;

            break;
        }
        case Token_Kind::UnaryOp: {
            const auto uop = uop_let(token_now->section_start);

            auto node_uop = new NodeUOp {
                pointer,
                parent,
                ASTNode_Kind::UOp,
                uop,
                nullptr,
            };

            {
                *pointer = node_uop;
            }

            {
                parent = node_uop;
                pointer = &node_uop->child;
            }

            ++token_gen_now;

            break;
        }
        case Token_Kind::BinaryOp: {
            auto target = *pointer;

            const BinaryOp biop = biop_let(token_now->section_start, token_now->section_end);
            const unsigned short biop_priority = biop_priority_let(biop);

            while (target->parent != nullptr && target->parent->kind == ASTNode_Kind::BiOp) {
                const auto target_parent = static_cast<NodeBiOp *>(target->parent);
                const unsigned short target_parent_priority = biop_priority_let(target_parent->operation);

                if (target_parent_priority >= biop_priority) {
                    target = target_parent;
                } else {
                    break;
                }
            }

            auto node_biop = new NodeBiOp {
                target->slot,
                target->parent,
                ASTNode_Kind::BiOp,
                biop,
                target,
                nullptr,
            };

            {
                *(target->slot) = node_biop;
                target->parent = node_biop;
                target->slot = &node_biop->left;
            }

            {
                parent = node_biop;
                pointer = &node_biop->right;
            }

            ++token_gen_now;

            break;
        }
        }
    }
}
