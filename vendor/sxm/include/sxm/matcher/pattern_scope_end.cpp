#include "sxm/matcher/pattern_scope_end.hpp"
#include "sxm/matcher/pattern.hpp"
#include "sxm/parser/token.hpp"
#include <optional>

std::optional<sxm::Pattern_Match> sxm::pattern_scope_end(const char *src, const Pattern_Context *context) {
    const auto scope = *(context->scope_stack.cend() - 1);

    switch (scope) {
    case Pattern_Scope::Flat: {
        if (*src == '\0') {
            return Pattern_Match{
                .token = {
                    .kind = Token_Kind::End,
                    .section_start = src,
                    .section_end = src,
                },

                .idx_next = Pattern_Kind::None,
            };
        }

        break;
    }
    case Pattern_Scope::Priority: {
        if (*src == ')') {
            return Pattern_Match{
                .token = {
                    .kind = Token_Kind::PriorityClose,
                    .section_start = src,
                    .section_end = src + 1,
                },

                .idx_next = Pattern_Kind::OperatorLit,
            };
        }

        break;
    }
    case Pattern_Scope::Fn: {
        switch (*src) {
        case ')':
            return Pattern_Match{
                .token = {
                    .kind = Token_Kind::FnClose,
                    .section_start = src,
                    .section_end = src + 1,
                },

                .idx_next = Pattern_Kind::OperatorLit,
            };
        case ',':
            return Pattern_Match{
                .token = {
                    .kind = Token_Kind::FnComma,
                    .section_start = src,
                    .section_end = src + 1,
                },

                .idx_next = Pattern_Kind::Symbol,
            };
        }

        break;
    }
    }


    return std::nullopt;
}
