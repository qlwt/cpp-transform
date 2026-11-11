#include "tr/point.hpp"
#include <format>

using namespace tr;

Point Point::mul(const Mx3 &left, const Point &right) {
    Point result;

    for (short row_i = 0; row_i < 3; ++row_i) {
        result.list[row_i] = right.list[0] * left.grid[row_i][0];
        result.list[row_i] += right.list[1] * left.grid[row_i][1];
        result.list[row_i] += right.list[2] * left.grid[row_i][2];
    }

    return result;
}

Point Point::mul_affine(const Mx3 &left, const Point &right) {
    Point result;

    result.list[0] = (
            + right.x() * left.a()
            + right.y() * left.c()
            + left.tx()
        );

    result.list[1] = (
            + right.x() * left.b()
            + right.y() * left.d()
            + left.ty()
        );

    result.list[2] = 1;

    return result;
}

std::string Point::tostr() {
    return std::format("({}, {}, {})", this->list[0], this->list[1], this->list[2]);
}
