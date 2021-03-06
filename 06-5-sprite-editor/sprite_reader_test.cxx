
#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "sprite_reader.hxx"

#include <iomanip>
#include <iostream>
#include <sstream>

TEST_CASE("save and load one sprite")
{
    using namespace std;

    sprite_reader loader;

    const rect     r{ { 0.3f, 0.3f }, { 0.5f, 0.5f } };
    const om::vec2 pos(0.123f, 0.123f);
    const om::vec2 size(1.0f, 0.3f);
    const float    angle{ 270 };
    const string   texture_name{ "tank.png" };
    const string   tank_name{ "tank_spr" };

    stringstream ss;
    ss << left << setw(12) << "id: " << tank_name << '\n';
    ss << left << setw(12) << "texture: " << texture_name << '\n';
    ss << left << setw(12) << "uv_rect: ";
    // clang-format off
    ss << left <<setprecision(3) << fixed
       << setw(7) << r.pos.x << ' '
       << setw(7) << r.pos.y << ' '
       << setw(7) << r.size.x << ' '
       << setw(7) << r.size.y << '\n';
    // clang-format on
    ss << left << setw(12) << "world_pos: " << pos.x << ' ' << pos.y << '\n';
    ss << left << setw(12) << "size: " << size.x << ' ' << size.y << '\n';
    ss << left << setw(12) << "angle: " << angle << '\n';

    std::unique_ptr<om::engine, void (*)(om::engine*)> e(om::create_engine(),
                                                         om::destroy_engine);

    om::engine&    texture_cache = *e;
    vector<sprite> sprites;
    loader.load_sprites(sprites, ss, texture_cache);

    cout << ss.str() << endl << "num_sprites: " << sprites.size() << endl;

    if (sprites.empty())
    {
        REQUIRE(false);
        return;
    }
    const sprite& spr_ref = sprites.at(0);
    sprite        spr(spr_ref);

    stringstream ssave;
    loader.save_sprites(sprites, ssave);
    // now load from just saved stream back
    vector<sprite> sprites_saved;
    loader.load_sprites(sprites_saved, ssave, texture_cache);

    REQUIRE(sprites.size() == 1);
    REQUIRE(spr.uv_rect() == r);
    REQUIRE(std::abs(spr.rotation() - angle) <= 0.000001f);
    REQUIRE(sprites_saved == sprites);
}
