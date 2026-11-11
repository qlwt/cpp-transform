#include <cstdint>
#include <format>
#include <string>
#include <fstream>
#include "tr/bmp.hpp"

using namespace tr;
using namespace img;

namespace {

inline void fout_write_u32(std::ofstream *fout, std::uint32_t number) {
    char bytes[4];

    bytes[0] = number & 0xFF;
    bytes[1] = (number >> 8) & 0xFF;
    bytes[2] = (number >> 16) & 0xFF;
    bytes[3] = (number >> 24) & 0xFF;

    fout->write(bytes, sizeof(bytes));
}

inline void fout_write_u16(std::ofstream *fout, std::uint16_t number) {
    char bytes[2];

    bytes[0] = number & 0xFF;
    bytes[1] = (number >> 8) & 0xFF;

    fout->write(bytes, sizeof(bytes));
}

}

std::string Pixel::tostr() {
    return std::format(
            "rgb({}, {}, {})",
            static_cast<int>(this->r),
            static_cast<int>(this->g),
            static_cast<int>(this->b)
        );
};

void BMPHeaderFile::write(std::ofstream *fout) {
    fout->write(this->magic, sizeof(this->magic));
    fout_write_u32(fout, this->filesize);
    fout_write_u32(fout, this->reserved);
    fout_write_u32(fout, this->pxoffset);
}

void BMPHeaderDIB::write(std::ofstream *fout) {
    fout_write_u32(fout, this->hsize);
    fout_write_u32(fout, this->width);
    fout_write_u32(fout, this->height);
    fout_write_u16(fout, this->planes);
    fout_write_u16(fout, this->depth);
    fout_write_u32(fout, this->compression);
    fout_write_u32(fout, this->gridsize);
    fout_write_u32(fout, this->res_x);
    fout_write_u32(fout, this->res_y);
    fout_write_u32(fout, this->palette);
    fout_write_u32(fout, this->colors);
}

void BMPGrid::write(std::ofstream *fout) {
    char bytes[3];

    for (std::uint32_t index = 0; index < this->size; ++index) {
        Pixel *px_now = this->list + index;

        bytes[0] = px_now->b;
        bytes[1] = px_now->g;
        bytes[2] = px_now->r;

        fout->write(bytes, sizeof(bytes));
    }
}

void BMP::write(std::ofstream *fout) {
    this->head_f.write(fout);
    this->head_dib.write(fout);

    {
        static constexpr char padding[3] { 0x00, 0x00, 0x00, };

        char bytes[3];
        short rowpad = (4 - (this->head_dib.width * 3) % 4) % 4;
        Pixel *px_now = this->grid.list + (this->head_dib.height - 1) * this->head_dib.width;

        for (std::uint32_t row_i = 0; row_i < this->head_dib.height; ++row_i) {
            for(std::uint32_t col_i = 0; col_i < this->head_dib.width; ++col_i) {
                bytes[0] = px_now->b;
                bytes[1] = px_now->g;
                bytes[2] = px_now->r;

                fout->write(bytes, sizeof(bytes));

                ++px_now;
            }

            fout->write(padding, rowpad);

            px_now -= 2 * this->head_dib.width;
        }
    }
}
