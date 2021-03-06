#pragma once

#include <istream>
#include <vector>

#include "sprite.hxx"

class sprite_reader
{
public:
    sprite_reader();

    void load_sprites(std::vector<sprite>&, std::istream& in,
                      om::engine& texture_cache);
    void save_sprites(const std::vector<sprite>&, std::ostream& out);
};
