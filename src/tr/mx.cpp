#include <cmath>
#include <format>
#include <initializer_list>
#include <string>
#include <utility>
#include "tr/mx.hpp"

using namespace tr;

namespace {

Mx3 mx_identity {
    .grid {
        { 1.0, 0.0, 0.0, },
        { 0.0, 1.0, 0.0, },
        { 0.0, 0.0, 1.0, },
    },
};

}

Mx3 Mx3::affine_interpret(Affine_Kind kind, double value) noexcept {
    switch (kind) {
    case Affine_Kind::Rotate: {
        return Mx3::affine_rotation(value);
    }
    case Affine_Kind::Mirror: {
        return Mx3::affine_mirror(value);
    }
    case Affine_Kind::Scale: {
        return Mx3::affine_scale(value, value);
    }
    case Affine_Kind::ScaleX: {
        return Mx3::affine_scale(value, 1.0);
    }
    case Affine_Kind::ScaleY: {
        return Mx3::affine_scale(1.0, value);
    }
    case Affine_Kind::Skew: {
        return Mx3::affine_skew(value, value);
    }
    case Affine_Kind::SkewX: {
        return Mx3::affine_skew(value, 0.0);
    }
    case Affine_Kind::SkewY: {
        return Mx3::affine_skew(0.0, value);
    }
    case Affine_Kind::Translate: {
        return Mx3::affine_translate(value, value);
    }
    case Affine_Kind::TranslateX: {
        return Mx3::affine_translate(value, 0.0);
    }
    case Affine_Kind::TranslateY: {
        return Mx3::affine_translate(0.0, value);
    }
    }

    std::unreachable();
}

Mx3 Mx3::affine_interpret_inverse(Affine_Kind kind, double value) noexcept {
    switch (kind) {
    case Affine_Kind::Rotate: {
        return Mx3::affine_rotation(-value);
    }
    case Affine_Kind::Mirror: {
        return Mx3::affine_mirror(-value);
    }
    case Affine_Kind::Scale: {
        return Mx3::affine_scale(1.0 / value, 1.0 / value);
    }
    case Affine_Kind::ScaleX: {
        return Mx3::affine_scale(1.0 / value, 1.0);
    }
    case Affine_Kind::ScaleY: {
        return Mx3::affine_scale(1.0, 1.0 / value);
    }
    case Affine_Kind::Skew: {
        return Mx3::affine_skew(-value, -value);
    }
    case Affine_Kind::SkewX: {
        return Mx3::affine_skew(-value, 0.0);
    }
    case Affine_Kind::SkewY: {
        return Mx3::affine_skew(0.0, -value);
    }
    case Affine_Kind::Translate: {
        return Mx3::affine_translate(-value, -value);
    }
    case Affine_Kind::TranslateX: {
        return Mx3::affine_translate(-value, 0.0);
    }
    case Affine_Kind::TranslateY: {
        return Mx3::affine_translate(0.0, -value);
    }
    }

    std::unreachable();
}

Mx3 Mx3::affine_skew(double x, double y) noexcept {
    return {
        .grid = {
            {
                1,
                x,
                0.0
            },
            {
                y,
                1,
                0.0
            },
            { 0.0, 0.0, 1.0 },
        },
    };
}

Mx3 Mx3::affine_mirror(double rad) noexcept {
    return {
        .grid = {
            {
                std::cos(rad * 2),
                std::sin(rad * 2),
                0.0
            },
            {
                std::sin(rad * 2),
                -std::cos(rad * 2),
                0.0
            },
            { 0.0, 0.0, 1.0 },
        },
    };
}

Mx3 Mx3::affine_rotation(double rad) noexcept {
    return {
        .grid = {
            {
                std::cos(rad),
                -std::sin(rad),
                0.0
            },
            {
                std::sin(rad),
                std::cos(rad),
                0.0
            },
            { 0.0, 0.0, 1.0 },
        },
    };
}

Mx3 Mx3::affine_translate(double translate_x, double translate_y) noexcept {
    return {
        .grid = {
            {
                1.0,
                0.0,
                translate_x,
            },
            {
                0.0,
                1.0,
                translate_y,
            },
            { 0.0, 0.0, 1.0 },
        },
    };
}

Mx3 Mx3::affine_scale(double factor_x, double factor_y) noexcept {
    return {
        .grid = {
            {
                factor_x,
                0.0,
                0.0,
            },
            {
                0.0,
                factor_y,
                0.0,
            },
            { 0.0, 0.0, 1.0 },
        },
    };
}

Mx3 Mx3::mul(const Mx3 &left, const Mx3 &right) noexcept {
    Mx3 result;

    for (short row_i = 0; row_i < 3; ++row_i) {
        for (short col_i = 0; col_i < 3; ++col_i) {
            result.grid[row_i][col_i] = (
                    + left.grid[row_i][0] * right.grid[0][col_i]
                    + left.grid[row_i][1] * right.grid[1][col_i]
                    + left.grid[row_i][2] * right.grid[2][col_i]
                );
        }
    }

    return result;
}

Mx3 Mx3::mul_affine(const Mx3 &left, const Mx3 &right) noexcept {
    Mx3 result;

    result.a() = left.a() * right.a() + left.c() * right.b();
    result.c() = left.a() * right.c() + left.c() * right.d();

    result.b() = left.b() * right.a() + left.d() * right.b();
    result.d() = left.b() * right.c() + left.d() * right.d();

    result.tx() = left.a() * right.tx() + left.c() * right.ty() + left.tx();
    result.ty() = left.b() * right.tx() + left.d() * right.ty() + left.ty();

    result.grid[2][0] = 0;
    result.grid[2][1] = 0;
    result.grid[2][2] = 1;

    return result;
}

Mx3 Mx3::mul(std::initializer_list<Mx3> mx_list) noexcept {
    Mx3 result = mx_identity;

    for (auto mx_it_now = mx_list.begin(); mx_it_now != mx_list.end(); ++mx_it_now) {
        result = Mx3::mul(result, *mx_it_now);
    }

    return result;
}

Mx3 Mx3::inverse(const Mx3 &src) noexcept {
    Mx3 result;

    double det = 1 / (
            + src.grid[0][0] * src.grid[1][1] * src.grid[2][2]
            + src.grid[0][1] * src.grid[1][2] * src.grid[2][0]
            + src.grid[0][2] * src.grid[1][0] * src.grid[2][1]
            - src.grid[0][2] * src.grid[1][1] * src.grid[2][0]
            - src.grid[0][0] * src.grid[1][2] * src.grid[2][1]
            - src.grid[0][1] * src.grid[1][0] * src.grid[2][2]
        );

    result.grid[0][0] = det * (src.grid[1][1] * src.grid[2][2] - src.grid[2][1] * src.grid[1][2]);
    result.grid[1][0] = det * -(src.grid[1][0] * src.grid[2][2] - src.grid[2][0] * src.grid[1][2]);
    result.grid[2][0] = det * (src.grid[1][0] * src.grid[2][1] - src.grid[2][0] * src.grid[1][1]);

    result.grid[0][1] = det * -(src.grid[0][1] * src.grid[2][2] - src.grid[2][1] * src.grid[0][2]);
    result.grid[1][1] = det * (src.grid[0][0] * src.grid[2][2] - src.grid[2][0] * src.grid[0][2]);
    result.grid[2][1] = det * -(src.grid[0][0] * src.grid[2][1] - src.grid[2][0] * src.grid[0][1]);

    result.grid[0][2] = det * (src.grid[0][1] * src.grid[1][2] - src.grid[1][1] * src.grid[0][2]);
    result.grid[1][2] = det * -(src.grid[0][0] * src.grid[1][2] - src.grid[1][0] * src.grid[0][2]);
    result.grid[2][2] = det * (src.grid[0][0] * src.grid[1][1] - src.grid[1][0] * src.grid[0][1]);

    return result;
}

Mx3 Mx3::inverse_affine(const Mx3 &src) noexcept {
    Mx3 result;

    double det = 1 / (src.a() * src.d() - src.b() * src.c());

    result.a() = src.d() * det;
    result.c() = -src.c() * det;
    result.b() = -src.b() * det;
    result.d() = src.a() * det;

    result.tx() = (src.c() * src.ty() - src.d() * src.tx()) * det;
    result.ty() = (src.b() * src.tx() - src.a() * src.ty()) * det;

    result.grid[2][0] = 0;
    result.grid[2][1] = 0;
    result.grid[2][2] = 1;

    return result;
}

std::string Mx3::tostr() noexcept {
    return std::format(
            "[({}, {}, {}), ({}, {}, {}), ({}, {}, {})]",
            this->grid[0][0],
            this->grid[0][1],
            this->grid[0][2],
            this->grid[1][0],
            this->grid[1][1],
            this->grid[1][2],
            this->grid[2][0],
            this->grid[2][1],
            this->grid[2][2]
        );
}
