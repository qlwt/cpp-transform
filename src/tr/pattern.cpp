#include "tr/pattern.hpp"
#include "tr/args.hpp"
#include "tr/bmp.hpp"
#include "tr/point.hpp"
#include <cmath>
#include <ranges>
#include <utility>
#include <vector>

using namespace tr;

namespace {

inline long mod_circ(long v, long base) {
    return (base + (v % base)) % base;
}

inline double sq(double src) {
    return src * src;
}

struct FramePat {
    virtual img::Pixel px_let(long x, long y) const = 0;
};

struct FramePatGrid : FramePat {
    std::uint32_t sq_size;

    FramePatGrid(std::uint32_t sq_size) : FramePat(), sq_size(sq_size) {}

    img::Pixel px_let(long x, long y) const override {
        long idx_row = (long)std::floor((double)y / this->sq_size);
        long idx_col = (long)std::floor((double)x / this->sq_size);

        switch(mod_circ(idx_row + idx_col, 3)) {
        case 0: {
            return { 0xff, 0x00, 0x00, };
        }
        case 1: {
            return { 0x00, 0xff, 0x00, };
        }
        case 2: {
            return { 0x00, 0x00, 0xff, };
        }
        default:
            std::unreachable();
        }
    };
};

struct FramePatGrad : FramePat {
    std::uint32_t screen_width;
    std::uint32_t screen_height;

    FramePatGrad(std::uint32_t screen_width, std::uint32_t screen_height)
        : FramePat(), screen_width(screen_width), screen_height(screen_height) {}

    FramePatGrad() : FramePat() {}

    img::Pixel px_let(long x, long y) const override {
        long width_anchor = this->screen_width >> 1;
        long height_anchor = this->screen_height >> 1;

        return {
            (std::uint8_t)std::abs(255 - mod_circ(x - width_anchor, 510)),
            (std::uint8_t)std::abs(255 - mod_circ(y - height_anchor, 510)),
            (std::uint8_t)std::abs(255 - mod_circ((x - width_anchor) * (y - height_anchor), 510)),
        };
    };
};

struct FramePatCross : FramePat {
    std::uint32_t cross_width;
    std::uint32_t screen_width;
    std::uint32_t screen_height;

    FramePatCross(std::uint32_t cross_width, std::uint32_t screen_width, std::uint32_t screen_height)
        : FramePat(), cross_width(cross_width), screen_width(screen_width), screen_height(screen_height) {}

    img::Pixel px_let(long x, long y) const override {
        long width_anchor = this->screen_width >> 1;
        long height_anchor = this->screen_height >> 1;

        if(x >= (width_anchor - this->cross_width) && x < (width_anchor + this->cross_width)) {
            return { 0x00, 0x00, 0xff, };
        }

        if(y >= (height_anchor - this->cross_width) && y < (height_anchor + this->cross_width)) {
            return { 0x00, 0x00, 0xff, };
        }

        return { 0xff, 0xff, 0xff, };
    };
};

inline img::Pixel px_let_final(double x, double y, Mx3 *mx_inv, FramePat *pattern) {
    Point point_mid = Point::mul_affine(*mx_inv, { x + 0.5, y + 0.5, 1 });
    Point point_target { point_mid.list[0] - 0.5, point_mid.list[1] - 0.5, 1 };

    long target_x = static_cast<long>(std::floor(point_target.list[0]));
    long target_y = static_cast<long>(std::floor(point_target.list[1]));

    double fract_x = point_target.list[0] - target_x;
    double fract_y = point_target.list[1] - target_y;
    double fract_ix = 1 - fract_x;
    double fract_iy = 1 - fract_y;

    img::Pixel target_tl = pattern->px_let(target_x, target_y);
    img::Pixel target_tr = pattern->px_let(target_x, target_y + 1);
    img::Pixel target_bl = pattern->px_let(target_x + 1, target_y);
    img::Pixel target_br = pattern->px_let(target_x + 1, target_y + 1);

    return img::Pixel {
        .r = static_cast<std::uint8_t>(
            + fract_ix * fract_iy * target_tl.r
            + fract_x * fract_iy * target_bl.r
            + fract_ix * fract_y * target_tr.r
            + fract_x * fract_y * target_br.r
        ),

        .g = static_cast<std::uint8_t>(
            + fract_ix * fract_iy * target_tl.g
            + fract_x * fract_iy * target_bl.g
            + fract_ix * fract_y * target_tr.g
            + fract_x * fract_y * target_br.g
        ),

        .b = static_cast<std::uint8_t>(
            + fract_ix * fract_iy * target_tl.b
            + fract_x * fract_iy * target_bl.b
            + fract_ix * fract_y * target_tr.b
            + fract_x * fract_y * target_br.b
        ),
    };
}

}

void tr::pattern_fill_precise(img::BMP *target, double time, ArgConfig *config) {
    std::vector<ArgTrUnit> trunit_list {};

    for (ArgTrDecl &trdecl : config->trdecl_list) {
        sxm::ceval::Unit unit = trdecl.decl.init();

        trunit_list.emplace_back(trdecl.kind, unit);
    }

    double screen_width = static_cast<double>(target->head_dib.width);
    double screen_height = static_cast<double>(target->head_dib.height);

    FramePatGrad pattern { config->screen_width, config->screen_height };
    Mx3 transform_center_sync = Mx3::affine_translate(-screen_width / 2.0, -screen_height / 2.0);
    Mx3 transform_center_back = Mx3::affine_translate(screen_width / 2.0, screen_height / 2.0);

    for (std::uint32_t row_i = 0; row_i < target->head_dib.height; ++row_i) {
        for (std::uint32_t col_i = 0; col_i < target->head_dib.width; ++col_i) {
            std::uint32_t index = row_i * target->head_dib.width + col_i;

            double px_x = static_cast<double>(col_i);
            double px_y = static_cast<double>(row_i);

            Mx3 transform = transform_center_back;

            for (ArgTrUnit &trunit : trunit_list | std::ranges::views::reverse) {
                Point origin = Point::mul_affine(transform, { 0.0, 0.0, 1.0 });

                double radius = std::sqrt(sq(screen_width / 2.0) + sq(screen_height / 2.0));
                double px_distance = std::sqrt(sq(px_x - origin.list[0]) + sq(px_y - origin.list[1]));
                double decay = std::exp(-std::log(5) * sq(px_distance / radius));

                trunit.unit.state_list.p_start[0] = { time };
                trunit.unit.state_list.p_start[1] = { px_x };
                trunit.unit.state_list.p_start[2] = { px_y };
                trunit.unit.state_list.p_start[3] = { origin.list[0] };
                trunit.unit.state_list.p_start[4] = { origin.list[1] };
                trunit.unit.state_list.p_start[5] = { decay };

                sxm::NumValue tr_param = trunit.unit.eval();

                Mx3 tr_now = Mx3::affine_interpret(trunit.kind, tr_param.real);

                transform = Mx3::mul_affine(transform, tr_now);
            }

            transform = Mx3::inverse_affine(Mx3::mul_affine(transform, transform_center_sync));
            target->grid.list[index] = px_let_final(px_x, px_y, &transform, &pattern);
        }
    }
}

void tr::pattern_fill_batch(img::BMP *target, double time, ArgConfig *config) {
    double screen_width = static_cast<double>(target->head_dib.width);
    double screen_height = static_cast<double>(target->head_dib.height);

    FramePatGrad pattern { config->screen_width, config->screen_height };

    Mx3 transform = Mx3::affine_translate(-screen_width / 2.0, -screen_height / 2.0);

    for (ArgTrDecl &trdecl : config->trdecl_list | std::views::reverse) {
        sxm::NumValue tr_param = trdecl.decl.eval({
            { time }
        });

        Mx3 tr_now = Mx3::affine_interpret_inverse(trdecl.kind, tr_param.real);

        transform = Mx3::mul_affine(tr_now, transform);
    }

    transform = Mx3::mul_affine(Mx3::affine_translate(screen_width / 2.0, screen_height / 2.0), transform);

    for (std::uint32_t row_i = 0; row_i < target->head_dib.height; ++row_i) {
        for (std::uint32_t col_i = 0; col_i < target->head_dib.width; ++col_i) {
            std::uint32_t index = row_i * target->head_dib.width + col_i;

            double px_x = static_cast<double>(col_i);
            double px_y = static_cast<double>(row_i);

            target->grid.list[index] = px_let_final(px_x, px_y, &transform, &pattern);
        }
    }
}
