#include "00_canvas.hxx"

irender::~irender() {}

bool operator==(const color& first, const color& second)
{
    return (first.r == second.r && first.g == second.g && first.b == second.b);
}

constexpr size_t color_size = sizeof(color);

static_assert(3 == color_size, "24 bit per pixel(r,g,b)");

position operator-(const position& left, const position& right)
{
    return { right.x - left.x, right.y - left.y };
}

bool operator==(const position& left, const position& right)
{
    return (left.x == right.x && left.y == right.y);
}
