#include "engine.hxx"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>

v0 blend_vertex(const v0& left, const v0& right, const float& t)
{
    v0 v;
    v.p.x = left.p.x + t * (right.p.x - left.p.x);
    v.p.y = left.p.y + t * (right.p.y - left.p.y);
    return v;
}

tri0 blend(const tri0& left, const tri0& right, const float& t)
{

    tri0 tr;
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
    texture* texture = engine->create_texture("tank.png");
    if (nullptr == texture)
    {
        std::cerr << "failed load texture\n";
        return EXIT_FAILURE;
    }

    int  current_shader = 2;
    bool continue_loop  = true;
    while (continue_loop)
    {
        int key = true;

        while (engine->read_input(key))
        {
            switch (key)
            {
                case -1:
                    continue_loop = false;
                    break;
                case 1:
                    ++current_shader;
                    if (current_shader > 2)
                    {
                        current_shader = 0;
                    }

                    break;
                default:
                    break;
            }
        }
        if (current_shader == 0)
        {
            std::ifstream file("vert_pos.txt");
            assert(!!file);

            tri0 tr1;
            tri0 tr2;
            tri0 tr11;
            tri0 tr22;

            file >> tr1 >> tr2 >> tr11 >> tr22;

            float time  = engine->get_time_from_init();
            float alpha = std::sin(time);

            tri0 t1 = blend(tr1, tr11, alpha);
            tri0 t2 = blend(tr2, tr22, alpha);

            engine->render(t1, color(1.f, 0.f, 0.f, 1.f));
            engine->render(t2, color(0.f, 1.f, 0.f, 1.f));
        }

        if (current_shader == 1)
        {
            std::ifstream file("vert_pos_color.txt");
            assert(!!file);

            tri1 tr1;
            tri1 tr2;

            file >> tr1 >> tr2;

            engine->render(tr1);
            engine->render(tr2);
        }

        if (current_shader == 2)
        {
            std::ifstream file("vert_tex_color.txt");
            assert(!!file);

            tri2 tr1;
            tri2 tr2;

            file >> tr1 >> tr2;

            float time = engine->get_time_from_init();
            float s    = std::sin(time);
            float c    = 1.f; // std::cos(time);

            // animate one triangle texture coordinates
            for (auto& v : tr1.v)
            {
                v.uv.u += c + 0.3f;
                v.uv.v += s;
            }

            engine->render(tr1, texture);
            engine->render(tr2, texture);
        }

        engine->swap_buffer();
    }

    engine->uninitialize();
    return EXIT_SUCCESS;
}
