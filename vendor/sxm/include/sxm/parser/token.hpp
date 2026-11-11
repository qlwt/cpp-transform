#pragma once

namespace sxm {

enum class Token_Kind {
    End,
    Space,
    NumberInt,
    NumberReal,
    Reference,
    BinaryOp,
    UnaryOp,
    PriorityOpen,
    PriorityClose,
    FnOpen,
    FnClose,
    FnComma,
};

struct Token {
  public:
    Token_Kind kind;
    const char *section_start;
    const char *section_end;
};

}
