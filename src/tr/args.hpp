#include "sxm/eval/compile/declaration.hpp"
#include "sxm/eval/compile/unit.hpp"
#include "tr/mx.hpp"
#include <filesystem>
#include <vector>

#pragma once

namespace tr {

struct ArgTrDecl {
    Affine_Kind kind;
    sxm::ceval::Declaration decl;

    ArgTrDecl() {};
    ArgTrDecl(Affine_Kind kind, sxm::ceval::Declaration &decl)
        : kind(kind), decl(std::move(decl)) {};

    ArgTrDecl(ArgTrDecl &&src) noexcept {
        this->decl = std::move(src.decl);
        this->kind = src.kind;
    };

    ArgTrDecl(const ArgTrDecl &src) noexcept = delete;
};

struct ArgTrUnit {
    Affine_Kind kind;
    sxm::ceval::Unit unit;

    ArgTrUnit() {};
    ArgTrUnit(Affine_Kind kind, sxm::ceval::Unit &unit)
        : kind(kind), unit(std::move(unit)) {};

    ArgTrUnit(ArgTrUnit &&src) noexcept {
        this->unit = std::move(src.unit);
        this->kind = src.kind;
    };

    ArgTrUnit(const ArgTrUnit &src) noexcept = delete;
};

struct ArgError {
    std::string message;
};

struct ArgConfig {
    bool flag_precise;
    std::filesystem::path dirpath {};
    unsigned int frames_amount {};
    unsigned int frames_ratio {};
    unsigned int screen_width {};
    unsigned int screen_height {};
    std::vector<ArgTrDecl> trdecl_list {};
};

std::optional<ArgError> args_parse(ArgConfig *config, int argc, char **argv);

}
