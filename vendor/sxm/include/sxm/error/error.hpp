#include <string>

#pragma once

namespace sxm {

enum class EvalError_Kind {
    FnEval,
    VarUndef,
    FnUndef,
    NodeUnexp,
};

struct EvalError {
    EvalError_Kind kind;
    std::string message;

    std::string tostr() const noexcept;
};

enum class ParserError_Kind {
    NoMatch,
    ScopeMismatch,
};

struct ParserError {
    ParserError_Kind kind;
    std::size_t pointer;
    const char *src;

    std::string tostr() const noexcept;
};

}
