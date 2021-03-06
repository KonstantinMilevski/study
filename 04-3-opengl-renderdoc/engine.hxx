#pragma once

#include <iosfwd>
#include <string>
#include <string_view>

struct vertex
{
    vertex()
        : x(0.0f)
        , y(0.0f)
        , z(0.0f)
        , r(0.0f)
        , g(0.0f)
        , b(0.0f)
    {
    }
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
};

struct triangle
{
    triangle()
    {
        v[0] = vertex();
        v[1] = vertex();
        v[2] = vertex();
    }
    vertex v[3];
};

std::istream& operator>>(std::istream& is, vertex& v);
std::istream& operator>>(std::istream& is, triangle& t);

class engine;

engine* create_engine();
void    destroy_engine(engine*);

class engine
{
public:
    ~engine();
    virtual std::string initialize(std::string_view) = 0;
    virtual void        uninitialize()               = 0;
    virtual void        render_triangle(triangle&)   = 0;
    virtual void        swap_buffer()                = 0;
    virtual bool        read_input(int&)             = 0;
};
