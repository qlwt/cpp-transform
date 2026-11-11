#include "sxm/matcher/pattern.hpp"
#include "sxm/matcher/pattern_ref.hpp"
#include "sxm/parser/token.hpp"
#include <optional>

namespace {

inline bool char_isalph(const char src) {
    const char src_lc = src | 0b00100000;

    return (src_lc >= 'a') && (src_lc <= 'z');
}

inline bool char_isnum(const char src) {
    return (src >= '0') && (src <= '9');
}

}

std::optional<sxm::Pattern_Match> sxm::pattern_ref(const char *src, const Pattern_Context *) {
    const char *src_now = src;

    if (char_isalph(*src_now) || *src_now == '_') {
        for (++src_now; char_isalph(*src_now) || char_isnum(*src_now) || *src_now == '_'; ++src_now);

        return Pattern_Match{
            .token = {
                .kind = Token_Kind::Reference,
                .section_start = src,
                .section_end = src_now,
            },

            .idx_next = Pattern_Kind::OperatorRef,
        };
    }

    return std::nullopt;
}
