#include "engine.hxx"
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>

vertex blend_vertex(const vertex& left, const vertex& right, const float& t)
{
    vertex v;
    v.x = left.x + t * (right.x - left.x);
    v.y = left.y + t * (right.y - left.y);
    return v;
}

triangle blend(const triangle& left, const triangle& right, const float& t)
{

    triangle tr;
    tr.v[0] = blend_vertex(left.v[0], right.v[0], t);
    tr.v[1] = blend_vertex(left.v[1], right.v[1], t);
    tr.v[2] = blend_vertex(left.v[2], right.v[2], t);
    return tr;
}

using namespace std;

int main()
{
    std::unique_ptr<engine, void (*)(engine*)> engine(create_engine(),
                                                      destroy_engine);

    const std::string error = engine->initialize("");
    if (!error.empty())
    {
        std::cerr << error << std::endl;
        return EXIT_FAILURE;
    }

    bool continue_loop = true;
    while (continue_loop)
    {
        bool key = true;

        while (engine->read_input(key))
        {
            switch (key)
            {
                case false:
                    continue_loop = false;
                    break;
                default:
                    break;
            }
        }
        std::ifstream file("vert_and_tex_coord.txt");
        assert(!!file);

        // float    alpha = (std::sin(engine->get_time_from_init()) * 0.5f) +
        // 0.5f;
        triangle t1, t2;

        file >> t1 >> t2;

        engine->render_triangle(t1);
        engine->render_triangle(t2);

        engine->swap_buffer();
    };

    engine->uninitialize();
    return EXIT_SUCCESS;
}
