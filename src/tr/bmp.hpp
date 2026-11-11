#include <cstdint>
#include <string>
#include <fstream>

#pragma once

namespace tr::img {

struct Pixel {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;

    std::string tostr();
};

struct BMPHeaderFile {
    char magic[2] { 0x42, 0x4D };
    std::uint32_t filesize;
    std::uint32_t reserved { 0 };
    std::uint32_t pxoffset { 54 };

    BMPHeaderFile(std::uint32_t filesize)
        : filesize(filesize) { }

    void write(std::ofstream *fout);
};

struct BMPHeaderDIB {
    std::uint32_t hsize { 40 };
    std::uint32_t width;
    std::uint32_t height;
    std::uint16_t planes { 1 };
    std::uint16_t depth { 24 };
    std::uint32_t compression { 0 };
    std::uint32_t gridsize;
    std::uint32_t res_x { 256 };
    std::uint32_t res_y { 256 };
    std::uint32_t palette { 0 };
    std::uint32_t colors { 0 };

    BMPHeaderDIB(std::uint32_t width, std::uint32_t height)
        : width(width), height(height), gridsize(3 * (width + width % 4) * height) { }

    void write(std::ofstream *fout);
};

struct BMPGrid {
    Pixel *list;
    std::uint32_t size;

    BMPGrid(std::uint32_t size)
        : list(new Pixel[size]), size(size) { }

    ~BMPGrid() {
        delete[] this->list;
    }

    void write(std::ofstream *fout);
};

struct BMP {
    BMPHeaderFile head_f;
    BMPHeaderDIB head_dib;
    BMPGrid grid;

    BMP(std::uint32_t width, std::uint32_t height)
        : head_f(54 + 3 * (width + width % 4) * height), head_dib(width, height), grid(width * height) { }

    void write(std::ofstream *fout);
};

}
