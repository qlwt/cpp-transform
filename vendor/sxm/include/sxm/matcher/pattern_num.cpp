#include "sxm/matcher/pattern.hpp"
#include "sxm/matcher/pattern_num.hpp"
#include "sxm/parser/token.hpp"
#include <optional>

namespace {

std::optional<sxm::Pattern_Match> eval_zero(const char *src, const char *src_now) {
    if (*src_now == '.') {
        for (++src_now;; ++src_now) {
            switch (*src_now) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                break;
            }
            default: {
                return sxm::Pattern_Match{
                    .token = {
                        .kind = sxm::Token_Kind::NumberReal,
                        .section_start = src,
                        .section_end = src_now,
                    },

                    .idx_next = sxm::Pattern_Kind::OperatorLit,
                };
            }
            }
        }
    }

    return sxm::Pattern_Match{
        .token = {
            .kind = sxm::Token_Kind::NumberInt,
            .section_start = src,
            .section_end = src_now,
        },

        .idx_next = sxm::Pattern_Kind::OperatorLit,
    };
}

std::optional<sxm::Pattern_Match> eval_tail(const char *src, const char *src_now) {
    if (*src_now == '-') {
        ++src_now;
    }

    for (;; ++src_now) {
        switch (*src_now) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            break;
        }
        default:
            return sxm::Pattern_Match{
                .token = {
                    .kind = sxm::Token_Kind::NumberReal,
                    .section_start = src,
                    .section_end = src_now,
                },

                .idx_next = sxm::Pattern_Kind::OperatorLit,
            };
        }
    }
}

std::optional<sxm::Pattern_Match> eval_point(const char *src, const char *src_now) {
    for (;; ++src_now) {
        switch (*src_now) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            break;
        }
        case 'e':
            return eval_tail(src, ++src_now);
        default:
            return sxm::Pattern_Match{
                .token = {
                    .kind = sxm::Token_Kind::NumberReal,
                    .section_start = src,
                    .section_end = src_now,
                },

                .idx_next = sxm::Pattern_Kind::OperatorLit,
            };
        }
    }
}

}

std::optional<sxm::Pattern_Match> sxm::pattern_num(const char *src, const sxm::Pattern_Context *) {
    const char *src_now = src;

    switch (*src_now) {
    case '0': {
        return eval_zero(src, ++src_now);
    }
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        break;
    default:
        return std::nullopt;
    }

    for (++src_now;; ++src_now) {
        switch (*src_now) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            break;
        }
        case 'e':
            return eval_tail(src, ++src_now);
        case '.': {
            return eval_point(src, ++src_now);
        }
        default: {
            return Pattern_Match{
                .token = {
                    .kind = Token_Kind::NumberInt,
                    .section_start = src,
                    .section_end = src_now,
                },

                .idx_next = Pattern_Kind::OperatorLit,
            };
        }
        }
    }

    return std::nullopt;
}
