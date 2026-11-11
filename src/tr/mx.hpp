#include <initializer_list>
#include <string>

#pragma once

namespace tr {

enum class Affine_Kind {
    Rotate,
    Translate,
    TranslateX,
    TranslateY,
    Scale,
    ScaleX,
    ScaleY,
    Mirror,
    Skew,
    SkewX,
    SkewY,
};

struct Mx3 {
    static Mx3 inverse(const Mx3 &src) noexcept;
    // simplified inverse for matrix representing affine transfomrations
    static Mx3 inverse_affine(const Mx3 &src) noexcept;

    static Mx3 mul(const Mx3 &left, const Mx3 &right) noexcept;
    static Mx3 mul(std::initializer_list<Mx3> mx_list) noexcept;
    // simpilfied multiply for matrix representing affine transformations
    static Mx3 mul_affine(const Mx3 &left, const Mx3 &right) noexcept;

    static Mx3 affine_interpret(Affine_Kind kind, double value) noexcept;
    static Mx3 affine_interpret_inverse(Affine_Kind kind, double value) noexcept;
    static Mx3 affine_mirror(double rad) noexcept;
    static Mx3 affine_rotation(double rad) noexcept;
    static Mx3 affine_skew(double x, double y) noexcept;
    static Mx3 affine_scale(double factor_x, double factor_y) noexcept;
    static Mx3 affine_translate(double translate_x, double translate_y) noexcept;

    double grid[3][3];

    std::string tostr() noexcept;

    inline const double &a() const noexcept {
        return this->grid[0][0];
    };

    inline double &a() noexcept {
        return this->grid[0][0];
    };

    inline const double &b() const noexcept {
        return this->grid[1][0];
    };

    inline double &b() noexcept {
        return this->grid[1][0];
    };

    inline const double &c() const noexcept {
        return this->grid[0][1];
    };

    inline double &c() noexcept {
        return this->grid[0][1];
    };

    inline const double &d() const noexcept {
        return this->grid[1][1];
    };

    inline double &d() noexcept {
        return this->grid[1][1];
    };

    inline const double &tx() const noexcept {
        return this->grid[0][2];
    };

    inline double &tx() noexcept {
        return this->grid[0][2];
    };

    inline const double &ty() const noexcept {
        return this->grid[1][2];
    };

    inline double &ty() noexcept {
        return this->grid[1][2];
    };
};

}
