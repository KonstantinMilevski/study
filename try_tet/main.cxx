﻿#include "engine.hxx"
#include "field.hxx"
#include "texture_gl_es20.hxx"

#include <algorithm>
#include <array>
#include <cassert>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>

std::array<std::array<size_t, 4>, 7> figures = {
    1, 3, 5, 7, // I
    2, 4, 5, 7, // S
    3, 5, 4, 6, // Z
    3, 5, 4, 7, // T
    2, 3, 5, 7, // L
    3, 5, 7, 6, // J
    2, 3, 4, 5, // O
};
/// left down texture origin point
std::array<vec2, 7> second_texture_pos = { {
    { 0.f, 0.f },
    { 1.f / 7, 0.f },
    { 2.f / 7, 0.f },
    { 3.f / 7, 0.f },
    { 4.f / 7, 0.f },
    { 5.f / 7, 0.f },
    { 6.f / 7, 0.f },
} };

std::array<size_t, 4> select_figure(std::array<std::array<size_t, 4>, 7>& fig)
{

    std::array<size_t, 4> res;
    srand(time(0));
    const size_t rand_index = static_cast<size_t>(rand());
    const size_t row        = rand_index % 7;
    for (size_t i = 0; i < 4; i++)
    {
        res.at(i) = fig[row][i];
    }
    return res;
}
rect generate_texture_color(std::array<vec2, 7>& text_pos)
{

    const size_t rand_index    = static_cast<size_t>(rand());
    const size_t number        = rand_index % 7;
    vec2         next_text_pos = text_pos.at(number);
    /// texture size
    vec2 next_text_size = { 1.f / 7, 1.f };
    return { next_text_pos, next_text_size };
}

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

    texture* text_main_bar = engine->create_texture("blocks1.png");

    if (nullptr == text_main_bar)
    {
        std::cerr << "failed load texture\n";
        return EXIT_FAILURE;
    }
    texture* b_g_texture = engine->create_texture("Back.png");
    if (nullptr == b_g_texture)
    {
        std::cerr << "failed load texture\n";
        return EXIT_FAILURE;
    }
    /// block start
    rect  block_pos({ 0.f, 0.f }, { cell_size, cell_size });
    rect  block_text({ 1.f / 7, 0.f }, { 1.f / 7, 1.f });
    block main_block(block_pos, block_text);
    block next_block(block_pos, block_text);

    /// field srart
    size_t width_main_field  = 10;
    size_t height_main_field = 20;
    field  main_field(width_main_field, height_main_field);
    /// field_next_figure
    size_t width_next_field  = 2;
    size_t height_next_field = 4;
    field  next_field(width_next_field, height_next_field);

    /// select first figure
    std::array<size_t, 4> start_coord = select_figure(figures);
    figure                start_figure(start_coord, width_main_field);
    figure                next_start_figure(start_coord, width_next_field); //
    next_field.set_figure(next_start_figure, main_block);                   //
    /// block background;
    rect                bg_block = { main_field.field_rect() };
    block               background(bg_block, { { 0.f, 0.f }, { 1.f, 1.f } });
    std::vector<vertex> back_vert = background.build_block();
    vertex_buffer*      back_vert_buf =
        engine->create_vertex_buffer(&back_vert[0], 6);

    ///
    std::vector<vertex> main_field_vert;
    vertex_buffer*      main_field_vert_buf;
    std::vector<vertex> next_field_vert;
    vertex_buffer*      next_field_vert_buf;

    int   d_pos       = 0;
    float start_timer = engine->get_time_from_init();
    float dt          = 0.25f;
    /// first figure start position '-4' row from top, center
    start_figure.figure_change_position(
        (height_main_field - 4) * width_main_field + width_main_field / 2);
    static bool rotate        = false;
    static bool continue_loop = true;
    static bool start_game    = true;
    static bool next          = true;
    static bool pause         = false;
    static rect next_texture; /// next texrure pos for random generation

    figure playing_figure; /// figure for main loop

    while (continue_loop)
    {
        if (start_game)
        {
            playing_figure = start_figure;

            start_game = false;
        }
        float current_time = engine->get_time_from_init();
        float timer        = current_time - start_timer;
        event game_event;
        while (engine->read_event(game_event))
        {
            switch (game_event.key)
            {
                case keys::exit:
                    continue_loop = false;
                    break;

                case keys::right:
                    if (engine->is_key_down(keys::right))
                    {
                        d_pos += 1;
                        break;
                    }
                case keys::left:
                    if (engine->is_key_down(keys::left))
                    {
                        d_pos -= 1;
                        break;
                    }
                case keys::rotate:
                    if (engine->is_key_down(keys::rotate))
                    {
                        rotate = true;
                        break;
                    }
                case keys::down:
                    if (engine->is_key_down(keys::down))
                    {
                        dt = 0.01f;
                        break;
                    }
                case keys::pause:
                    if (engine->is_key_down(keys::pause))
                    {
                        pause == false ? pause = true : pause = false;
                        break;
                    }

                default:
                    break;
            }
        }
        if (pause)
        {
            continue;
        }
        figure prev = playing_figure;
        main_field.clear_position(playing_figure);

        playing_figure.figure_change_position(d_pos);
        if (!(main_field.check_field_border(playing_figure) &&
              main_field.check_empty_cell(playing_figure) &&
              main_field.check_figure_horizont(prev, playing_figure)))
        {
            playing_figure = prev;
        }

        if (rotate)
        {
            prev = playing_figure;
            main_field.clear_position(playing_figure);
            playing_figure.figure_rotate(width_main_field);
            if (!(main_field.check_field_border(playing_figure) &&
                  main_field.check_empty_cell(playing_figure)))
            {
                playing_figure = prev;
            }
        }

        main_field.set_figure(playing_figure, main_block);
        figure                next_figure;
        figure                next_figure_view;
        std::array<size_t, 4> next_coord;

        if (next)
        {
            next_texture = generate_texture_color(second_texture_pos);
            next_coord   = select_figure(figures);
            next_block.set_texture_pos(next_texture);
            next = false;
        }

        if (timer > dt)
        {
            prev  = playing_figure;
            d_pos = -width_main_field;
            main_field.clear_position(playing_figure);
            playing_figure.figure_change_position(d_pos);

            if (!next)
            {
                next_field.clear_field();
            }
            next_figure_view = { next_coord, width_next_field };
            next_field.set_figure(next_figure_view, next_block);

            if (main_field.check_field_border(playing_figure) &&
                main_field.check_empty_cell(playing_figure))
            {
                main_field.set_figure(playing_figure, main_block);
            }
            else
            {
                main_field.set_figure(prev, main_block);
                next_figure = { next_coord, width_main_field };
                main_block.set_texture_pos(next_texture);
                playing_figure = next_figure;
                playing_figure.figure_change_position((height_main_field - 4) *
                                                          width_main_field +
                                                      width_main_field / 2);
                main_field.check_field();
                if (prev.compare_position(playing_figure))
                {
                    std::cerr << "Game OVer";
                    break;
                }
                main_field.set_figure(playing_figure, main_block);
                next = true;
            }

            start_timer = current_time;
        }

        //
        next_field_vert     = next_field.occupied_cells();
        next_field_vert_buf = engine->create_vertex_buffer(
            &next_field_vert[0], next_field_vert.size());
        //
        main_field_vert     = main_field.occupied_cells();
        main_field_vert_buf = engine->create_vertex_buffer(
            &main_field_vert[0], main_field_vert.size());
        rotate = false;
        dt     = 0.4;
        d_pos  = 0;

        /// back
        vec2   bg_zero_pos(-45, -96);
        matrix bg_zero          = matrix::move(bg_zero_pos);
        matrix bg_screen_aspect = matrix::scale(window_scale, 1.0f);
        matrix bg_size_scale    = matrix::scale(0.0128f, 0.0103f);
        matrix bg_m             = bg_zero * bg_screen_aspect * bg_size_scale;
        engine->render_tet(*back_vert_buf, b_g_texture, bg_m);

        /// main field
        vec2   zero_pos(cell_size * field_width * (-0.5),
                      cell_size * field_height * (-0.5));
        matrix zero          = matrix::move(zero_pos);
        matrix screen_aspect = matrix::scale(window_scale, 1.0f);
        matrix size_scale    = matrix::scale(0.009f, 0.009f);
        matrix m             = zero * screen_aspect * size_scale;
        engine->render_tet(*main_field_vert_buf, text_main_bar, m);
        /// next field
        vec2   next_f_zero_pos(110, 100);
        matrix next_f_zero = matrix::move(next_f_zero_pos);
        matrix next_f_screen_aspect =
            matrix::scale(window_scale * 0.006f, 1.0f * 0.006f);
        matrix next_f_m = next_f_zero * next_f_screen_aspect;
        engine->render_tet(*next_field_vert_buf, text_main_bar, next_f_m);

        ///
        engine->swap_buffer();
        engine->destroy_vertex_buffer(main_field_vert_buf);
        engine->destroy_vertex_buffer(next_field_vert_buf);
    }
    engine->destroy_vertex_buffer(back_vert_buf);
    engine->destroy_texture(text_main_bar);
    engine->destroy_texture(b_g_texture);
    engine->uninitialize();
    return 0;
}
