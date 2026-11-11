#include "sxm/matcher/pattern_biop.hpp"
#include "sxm/matcher/pattern.hpp"
#include "sxm/matcher/pattern.hpp"
#include "sxm/parser/token.hpp"
#include <optional>

std::optional<sxm::Pattern_Match> sxm::pattern_biop(const char *src, const Pattern_Context *) {
    switch (*src) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '^':
        return Pattern_Match{
            .token = {
                .kind = Token_Kind::BinaryOp,
                .section_start = src,
                .section_end = src + 1,
            },

            .idx_next = Pattern_Kind::Symbol,
        };
    case '>':
    case '<':
    case '=':
        if (*(src + 1) == '=') {
            return Pattern_Match{
                .token = {
                    .kind = Token_Kind::BinaryOp,
                    .section_start = src,
                    .section_end = src + 2,
                },

                .idx_next = Pattern_Kind::Symbol,
            };
        }

        return Pattern_Match{
            .token = {
                .kind = Token_Kind::BinaryOp,
                .section_start = src,
                .section_end = src + 1,
            },

            .idx_next = Pattern_Kind::Symbol,
        };
    case '&':
        if (*(src + 1) == '&') {
            return Pattern_Match {
                .token = {
                    .kind = Token_Kind::BinaryOp,
                    .section_start = src,
                    .section_end = src + 2,
                },

                .idx_next = Pattern_Kind::Symbol,
            };
        }

        return std::nullopt;
    case '|':
        if (*(src + 1) == '|') {
            return Pattern_Match {
                .token = {
                    .kind = Token_Kind::BinaryOp,
                    .section_start = src,
                    .section_end = src + 2,
                },

                .idx_next = Pattern_Kind::Symbol,
            };
        }

        return std::nullopt;
    case '!':
        if (*(src + 1) == '=') {
            return Pattern_Match{
                .token = {
                    .kind = Token_Kind::BinaryOp,
                    .section_start = src,
                    .section_end = src + 2,
                },

                .idx_next = Pattern_Kind::Symbol,
            };
        }

        return std::nullopt;
    }

    return std::nullopt;
}
