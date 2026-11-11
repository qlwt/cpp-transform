#include "sxm/parser/parse.hpp"
#include "sxm/matcher/pattern_biop.hpp"
#include "sxm/matcher/pattern_uop.hpp"
#include "sxm/matcher/pattern_charseq.hpp"
#include "sxm/matcher/pattern_scope_end.hpp"
#include "sxm/matcher/pattern_num.hpp"
#include "sxm/matcher/pattern_ref.hpp"
#include "sxm/matcher/pattern_race.hpp"
#include "sxm/matcher/pattern_space.hpp"
#include "sxm/matcher/pattern.hpp"
#include "sxm/parser/token.hpp"
#include <array>
#include <cstddef>
#include <generator>
#include <vector>
#include <expected>

namespace {

const sxm::Pattern pattern_none = sxm::pattern_race<0, std::array<sxm::Pattern, 0> {}>;

const sxm::Pattern pattern_symbol = sxm::pattern_race<5, std::array<sxm::Pattern, 5> {
    sxm::pattern_space<sxm::Pattern_Kind::Symbol>,
    sxm::pattern_uop<sxm::Pattern_Kind::Symbol>,
    sxm::pattern_num,
    sxm::pattern_ref,
    sxm::pattern_charseq<1, std::array<char, 1> { '(', }, sxm::Token_Kind::PriorityOpen, sxm::Pattern_Kind::Symbol >,
} >;

const sxm::Pattern pattern_fnempty = sxm::pattern_race<6, std::array<sxm::Pattern, 6> {
    sxm::pattern_space<sxm::Pattern_Kind::FnEmpty>,
    sxm::pattern_uop<sxm::Pattern_Kind::FnEmpty>,
    sxm::pattern_num,
    sxm::pattern_ref,
    sxm::pattern_charseq<1, std::array<char, 1> { '(', }, sxm::Token_Kind::PriorityOpen, sxm::Pattern_Kind::Symbol >,
    sxm::pattern_charseq<1, std::array<char, 1> { ')', }, sxm::Token_Kind::FnClose, sxm::Pattern_Kind::OperatorLit >,
} >;

const sxm::Pattern pattern_operator_lit = sxm::pattern_race<3, std::array<sxm::Pattern, 3> {
    sxm::pattern_space<sxm::Pattern_Kind::OperatorLit>,
    sxm::pattern_biop,
    sxm::pattern_scope_end,
} >;

const sxm::Pattern pattern_operator_ref = sxm::pattern_race<4, std::array<sxm::Pattern, 4> {
    sxm::pattern_space<sxm::Pattern_Kind::OperatorRef>,
    sxm::pattern_biop,
    sxm::pattern_charseq<1, std::array<char, 1> { '(', }, sxm::Token_Kind::FnOpen, sxm::Pattern_Kind::FnEmpty >,
    sxm::pattern_scope_end,
} >;

inline sxm::Pattern pattern_let(sxm::Pattern_Kind kind) {
    switch (kind) {
    case sxm::Pattern_Kind::OperatorLit: {
        return pattern_operator_lit;
    }
    case sxm::Pattern_Kind::OperatorRef: {
        return pattern_operator_ref;
    }
    case sxm::Pattern_Kind::Symbol: {
        return pattern_symbol;
    }
    case sxm::Pattern_Kind::FnEmpty: {
        return pattern_fnempty;
    }
    case sxm::Pattern_Kind::None: {
        return pattern_none;
    }
    }

    return pattern_none;
}

} // namespace

std::generator<std::expected<sxm::Token, sxm::ParserError> &> sxm::parse(const char *src) {
    const char *src_now = src;

    Pattern_Context context {
        .scope_stack = std::vector { Pattern_Scope::Flat },
    };

    Pattern_Kind pattern_kind {
        Pattern_Kind::Symbol
    };

    std::expected<sxm::Token, sxm::ParserError> now;

    while (pattern_kind != Pattern_Kind::None) {
        const auto pattern = pattern_let(pattern_kind);
        const auto pattern_match_o = pattern(src_now, &context);

        if (pattern_match_o.has_value()) {
            const auto pattern_match = pattern_match_o.value();

            switch (pattern_match.token.kind) {
            case Token_Kind::Space:
                break;
            case Token_Kind::FnClose:
            case Token_Kind::PriorityClose: {
                context.scope_stack.pop_back();

                now = pattern_match.token;

                co_yield now;
                break;
            }
            case Token_Kind::PriorityOpen: {
                context.scope_stack.emplace_back(Pattern_Scope::Priority);
                now = pattern_match.token;

                co_yield now;
                break;
            }
            case Token_Kind::FnOpen: {
                context.scope_stack.emplace_back(Pattern_Scope::Fn);
                now = pattern_match.token;

                co_yield now;
                break;
            }
            case Token_Kind::UnaryOp: {
                if (*pattern_match.token.section_start == '+') {
                    break;
                }

                now = pattern_match.token;

                co_yield now;
                break;
            }
            default: {
                now = pattern_match.token;

                co_yield now;
                break;
            }
            }

            pattern_kind = pattern_match.idx_next;
            src_now = pattern_match.token.section_end;
        } else {
            now = std::unexpected(ParserError {
                .kind = ParserError_Kind::NoMatch,
                .pointer = static_cast<std::size_t>(src_now - src),
                .src = src,
            });

            co_yield now;
        }
    }

    if (context.scope_stack.size() > 1) {
        now = std::unexpected(ParserError {
            .kind = ParserError_Kind::ScopeMismatch,
            .pointer = static_cast<std::size_t>(src_now - src),
            .src = src,
        });

        co_yield now;
    }
}
