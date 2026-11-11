#include "tr/args.hpp"
#include "sxm/eval/compile/build.hpp"
#include "sxm/util/search_map/h_static.hpp"
#include "tr/mx.hpp"
#include <expected>
#include <format>
#include <optional>
#include <utility>

namespace {

enum class Expectation {
    Target,
    Repeats,
    Screen,
    Param
};

enum Param_Kind {
    FlagPrecise,
    Rotate,
    Skew,
    SkewY,
    SkewX,
    Mirror,
    MirrorX,
    MirrorY,
    Scale,
    ScaleX,
    ScaleY,
    Translate,
    TranslateX,
    TranslateY,
};

inline std::optional<tr::ArgError> parse_repeats(tr::ArgConfig *config, const char *arg) {
    const char *delim_end;
    const char *delim_idx {};

    for (const char *arg_now = arg;;) {
        if (*arg_now == '\0') {
            delim_end = arg_now;

            break;
        }

        if (*arg_now == '/') {
            delim_idx = arg_now;
        }

        ++arg_now;
    }

    if (delim_idx == nullptr) {
        return tr::ArgError {
            "Could not find delimiter in frame-amount positional parameter. Expected pattern 240/24",
        };
    }

    auto parser_amount = std::from_chars(arg, delim_idx, config->frames_amount);
    auto parser_ratio = std::from_chars(delim_idx + 1, delim_end, config->frames_ratio);

    if (parser_amount.ec != std::errc {} || parser_ratio.ec != std::errc {}) {
        return tr::ArgError {
            "Could not parse integers in frame-amount positional parameter. Expected pattern 240/24",
        };
    }

    return std::nullopt;
}

inline std::optional<tr::ArgError> parse_screen(tr::ArgConfig *config, const char *arg) {
    const char *delim_end;
    const char *delim_idx {};

    for (const char *arg_now = arg;;) {
        if (*arg_now == '\0') {
            delim_end = arg_now;

            break;
        }

        if (*arg_now == 'x') {
            delim_idx = arg_now;
        }

        ++arg_now;
    }

    if (delim_idx == nullptr) {
        return tr::ArgError {
            "Could not find delimiter in screen-size positional parameter. Expected pattern 854x480",
        };
    }

    auto parser_width = std::from_chars(arg, delim_idx, config->screen_width);
    auto parser_height = std::from_chars(delim_idx + 1, delim_end, config->screen_height);

    if (parser_width.ec != std::errc {} || parser_height.ec != std::errc {}) {
        return tr::ArgError {
            "Could not find delimiter in screen-size positional parameter. Expected pattern 854x480",
        };
    }

    return std::nullopt;
}

struct Param {
    Param_Kind kind;
    std::string pattern;
};

constexpr const Param param_list_start[] {
    {
        Param_Kind::FlagPrecise,
        "--precise"
    },
    {
        Param_Kind::Mirror,
        "--mirror"
    },
    {
        Param_Kind::MirrorX,
        "--mirror-x"
    },
    {
        Param_Kind::MirrorY,
        "--mirror-y"
    },
    {
        Param_Kind::Rotate,
        "--rotate"
    },
    {
        Param_Kind::Scale,
        "--scale"
    },
    {
        Param_Kind::ScaleX,
        "--scale-x"
    },
    {
        Param_Kind::ScaleY,
        "--scale-y"
    },
    {
        Param_Kind::Skew,
        "--skew"
    },
    {
        Param_Kind::SkewX,
        "--skew-x"
    },
    {
        Param_Kind::SkewY,
        "--skew-y"
    },
    {
        Param_Kind::Translate,
        "--translate"
    },
    {
        Param_Kind::TranslateX,
        "--translate-x"
    },
    {
        Param_Kind::TranslateY,
        "--translate-y"
    },
};

constexpr const Param *param_list_end {
    param_list_start + sizeof(param_list_start) / sizeof(Param)
};

std::optional<Param_Kind> param_kind_find(const char *arg) {
    for (const Param *param_now = param_list_start; ; ++param_now) {
        if (param_now >= param_list_end) {
            return std::nullopt;
        }

        if (param_now->pattern == arg) {
            return param_now->kind;
        }
    }
}

tr::Affine_Kind mxgen_kind_find(Param_Kind param_kind) {
    switch (param_kind) {
    case Param_Kind::FlagPrecise: {
        std::unreachable();
    }
    case Param_Kind::Rotate: {
        return tr::Affine_Kind::Rotate;
    }
    case Param_Kind::Skew: {
        return tr::Affine_Kind::Skew;
    }
    case Param_Kind::SkewY: {
        return tr::Affine_Kind::SkewY;
    }
    case Param_Kind::SkewX: {
        return tr::Affine_Kind::SkewX;
    }
    case Param_Kind::Mirror:
    case Param_Kind::MirrorY:
    case Param_Kind::MirrorX: {
        return tr::Affine_Kind::Mirror;
    }
    case Param_Kind::Scale: {
        return tr::Affine_Kind::Scale;
    }
    case Param_Kind::ScaleX: {
        return tr::Affine_Kind::ScaleX;
    }
    case Param_Kind::ScaleY: {
        return tr::Affine_Kind::ScaleY;
    }
    case Param_Kind::Translate: {
        return tr::Affine_Kind::Translate;
    }
    case Param_Kind::TranslateX: {
        return tr::Affine_Kind::TranslateX;
    }
    case Param_Kind::TranslateY: {
        return tr::Affine_Kind::TranslateY;
    }
    }

    std::unreachable();
}

}

std::optional<tr::ArgError> tr::args_parse(ArgConfig *config, int arg_size, char **arg_start) {
    sxm::HMapStatic<16, sxm::NumValue> varmap_const;
    sxm::HMapStatic<16, sxm::ceval::Offset> varmap_state;

    varmap_const.insert({ "e" }, { std::numbers::e });
    varmap_const.insert({ "pi" }, { std::numbers::pi });
    varmap_const.insert({ "phi" }, { std::numbers::phi });

    varmap_state.insert({ "time" }, { 0 });

    char **arg_end = arg_start + arg_size;
    Expectation expectation { Expectation::Target };

    for (char **arg_now = arg_start + 1; arg_now < arg_end; ++arg_now) {
        char *arg = *arg_now;

        switch (expectation) {
        case Expectation::Target: {
            config->dirpath = std::filesystem::path {
                std::u8string { reinterpret_cast<char8_t*>(arg) }
            };

            expectation = Expectation::Repeats;
            break;
        }
        case Expectation::Repeats: {
            parse_repeats(config, arg);

            expectation = Expectation::Screen;
            break;
        }
        case Expectation::Screen: {
            parse_screen(config, arg);

            varmap_const.insert({ "width" }, { static_cast<double>(config->screen_width) });
            varmap_const.insert({ "height" }, { static_cast<double>(config->screen_height) });

            expectation = Expectation::Param;
            break;
        }
        case Expectation::Param: {
            std::optional<Param_Kind> param_kind = param_kind_find(arg);

            if (!param_kind.has_value()) {
                return tr::ArgError {
                    std::format("Unknown parameter {}", arg),
                };
            }

            switch (*param_kind) {
            case Param_Kind::FlagPrecise: {
                config->flag_precise = true;

                varmap_state.insert({ "px_x" }, { 1 });
                varmap_state.insert({ "px_y" }, { 2 });
                varmap_state.insert({ "origin_x" }, { 3 });
                varmap_state.insert({ "origin_y" }, { 4 });
                varmap_state.insert({ "decay" }, { 5 });

                break;
            }
            case Param_Kind::MirrorX: {
                auto decl = sxm::ceval::build("pi / 2", {
                    .varmap_const = varmap_const,
                    .varmap_state = varmap_state,
                    .varmap_state_size = 6,
                });

                config->trdecl_list.emplace_back(tr::Affine_Kind::Mirror, decl.value());

                break;
            }
            case Param_Kind::MirrorY: {
                auto decl = sxm::ceval::build("0", {
                    .varmap_const = varmap_const,
                    .varmap_state = varmap_state,
                    .varmap_state_size = 6,
                });

                config->trdecl_list.emplace_back(tr::Affine_Kind::Mirror, decl.value());

                break;
            }
            case Param_Kind::Rotate:
            case Param_Kind::Scale:
            case Param_Kind::ScaleX:
            case Param_Kind::ScaleY:
            case Param_Kind::Skew:
            case Param_Kind::SkewX:
            case Param_Kind::SkewY:
            case Param_Kind::Translate:
            case Param_Kind::TranslateX:
            case Param_Kind::TranslateY:
            case Param_Kind::Mirror: {
                if (arg_end - arg_now < 2) {
                    return tr::ArgError {
                        std::format("No value provided for {}", arg)
                    };
                }

                auto decl = sxm::ceval::build(*(arg_now + 1), {
                    .varmap_const = varmap_const,
                    .varmap_state = varmap_state,
                    .varmap_state_size = 6,
                });

                if (!decl.has_value()) {
                    return std::visit([&](const auto & error) -> tr::ArgError {
                        using T = std::decay_t<decltype(error)>;

                        if constexpr (std::is_same_v<T, sxm::ParserError>) {
                            return tr::ArgError {
                                std::format("Could not parse expression for {}: {}", arg, error.tostr())
                            };
                        } else if constexpr (std::is_same_v<T, sxm::EvalError>) {
                            return tr::ArgError {
                                std::format("Could not parse expression for {}: {}", arg, error.tostr())
                            };
                        }
                    }, decl.error());
                }

                config->trdecl_list.emplace_back(mxgen_kind_find(*param_kind), decl.value());

                ++arg_now;
                break;
            }
            }
        }
        }
    }

    if (expectation != Expectation::Param) {
        return tr::ArgError {
            "Not enough positional arguments provided"
        };
    }

    if (config->frames_amount == 0 || config->frames_ratio == 0) {
        return tr::ArgError {
            "Frame amount and Frames per second are both expected to be >= 1"
        };
    }

    if (config->screen_width == 0 || config->screen_height == 0) {
        return tr::ArgError {
            "Screen width and height are both expected to be >= 1"
        };
    }

    return std::nullopt;
}
