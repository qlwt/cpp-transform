#include "sxm/parser/token.hpp"
#include <optional>
#include <vector>

#pragma once

namespace sxm {

enum class Pattern_Scope {
    Flat,
    Fn,
    Priority,
};

struct Pattern_Context {
    std::vector<Pattern_Scope> scope_stack;
};

enum class Pattern_Kind {
    None,
    Symbol,
    FnEmpty,
    OperatorLit,
    OperatorRef,
};

struct Pattern_Match {
    Token token;
    Pattern_Kind idx_next;
};

using Pattern = std::optional<Pattern_Match> (*)(const char *pointer, const Pattern_Context *context);

}
