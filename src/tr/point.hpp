#include "tr/mx.hpp"
#include <string>

#pragma once

namespace tr {

struct Point {
    static Point mul(const Mx3 &left, const Point &right);
    static Point mul_affine(const Mx3 &left, const Point &right);

    double list[3];

    std::string tostr();

    inline double x() const noexcept {
        return this->list[0];
    }

    inline double y() const noexcept {
        return this->list[1];
    }
};

}
