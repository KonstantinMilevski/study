#pragma once

#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

constexpr size_t width  = 3;
constexpr size_t height = 4;

#pragma pack(push, 1)
struct color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};
#pragma pack(pop)
bool operator==(const color&, const color&);

struct position
{
    int32_t x;
    int32_t y;
};
position operator-(const position&, const position&);

using pixels_vector = std::vector<position>;

const size_t buffer_size = height * width;

struct canvas : std::array<color, buffer_size>
{

    void save_image(const std::string& file_name)
    {

        std::ofstream out_file;
        out_file.exceptions(std::ios_base::failbit);
        out_file.open(file_name, std::ios_base::binary);
        out_file << "P6\n" << width << ' ' << height << ' ' << 255 << '\n';
        std::streamsize buf_size =
            static_cast<std::streamsize>(sizeof(color) * this->size());
        out_file.write(reinterpret_cast<const char*>(this), buf_size);
        out_file.close();
    }
    void load_image(const std::string file_name)
    {
        std::ifstream in_file;
        in_file.exceptions(std::ios_base::failbit);
        in_file.open(file_name, std::ios_base::binary);
        std::string header;
        size_t      image_width  = 0;
        size_t      image_height = 0;
        std::string color_format;
        in_file >> header >> image_width >> image_height >> color_format >>
            std::ws;
        if (size() != image_height * image_width)
        {
            throw std::runtime_error("image size not match");
        }

        std::streamsize buf_size =
            static_cast<std::streamsize>(sizeof(color) * this->size());
        in_file.read(reinterpret_cast<char*>(this), buf_size);
        in_file.close();
    }
};
struct irender
{
    virtual void          clear(color)                                   = 0;
    virtual void          set_pixel(position, color)                     = 0;
    virtual pixels_vector pixel_position(position& start, position& end) = 0;
    virtual ~irender();
};
