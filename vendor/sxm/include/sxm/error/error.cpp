#include "sxm/error/error.hpp"
#include <format>
#include <utility>

using namespace sxm;

std::string EvalError::tostr() const noexcept {
    return this->message;
}

std::string ParserError::tostr() const noexcept {
    std::string res {};

    switch (this->kind) {
    case sxm::ParserError_Kind::NoMatch: {
        res += std::format("No Match at position {}", this->pointer);

        break;
    }
    case sxm::ParserError_Kind::ScopeMismatch: {
        res += std::format("Scope Mismatch at position {}", this->pointer);

        break;
    }
    default: {
        std::unreachable();
    }
    }

    {
        res += " \"";

        if (this->pointer >= 5) {
            res += std::format("{}", std::string_view { this->src + this->pointer - 5, this->src + this->pointer });
        } else {
            res += std::format("{}", std::string_view  { this->src, this->src + this->pointer });
        }

        res += std::format("${}", std::string_view { src + this->pointer, src +this->pointer + 10 });
    }

    return res;
}
