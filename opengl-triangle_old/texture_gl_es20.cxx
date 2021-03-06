#include "texture_gl_es20.hxx"
#include "picopng.hxx"

/// texture impl

texture_gl_es20::texture_gl_es20(std::string_view path)
    : file_path(path)
{
    std::vector<std::byte> png_file_in_memory;
    std::ifstream          ifs(path.data(), std::ios_base::binary);
    if (!ifs)
    {
        throw std::runtime_error("can't load texture");
    }
    ifs.seekg(0, std::ios_base::end);
    std::streamoff pos_in_file = ifs.tellg();
    png_file_in_memory.resize(static_cast<size_t>(pos_in_file));
    ifs.seekg(0, std::ios_base::beg);
    if (!ifs)
    {
        throw std::runtime_error("can't load texture");
    }

    ifs.read(reinterpret_cast<char*>(png_file_in_memory.data()), pos_in_file);
    if (!ifs.good())
    {
        throw std::runtime_error("can't load texture");
    }

    std::vector<std::byte> image;
    unsigned long          w = 0;
    unsigned long          h = 0;
    // clang-format off
    int error = decodePNG(
    image,
    w,
    h,
    &png_file_in_memory[0],
    png_file_in_memory.size(),
    false);
    // clang-format on

    // if there's an error, display it
    if (error != 0)
    {
        std::cerr << "error: " << error << std::endl;
        throw std::runtime_error("can't load texture");
    }
    this->tex_width  = static_cast<uint32_t>(w);
    this->tex_height = static_cast<uint32_t>(h);

    gen_texture_from_pixels(image.data(), static_cast<size_t>(w),
                            static_cast<size_t>(h));
}

texture_gl_es20::texture_gl_es20(std::string_view path, const size_t w_new,
                                 const size_t h_new)
    : file_path(path)
{
    //    std::vector<std::byte> png_file_in_memory;
    //    std::ifstream          ifs(path.data(), std::ios_base::binary);
    //    if (!ifs)
    //    {
    //        throw std::runtime_error("can't load texture");
    //    }
    //    ifs.seekg(0, std::ios_base::end);
    //    std::streamoff pos_in_file = ifs.tellg();
    //    png_file_in_memory.resize(static_cast<size_t>(pos_in_file));
    //    ifs.seekg(0, std::ios_base::beg);
    //    if (!ifs)
    //    {
    //        throw std::runtime_error("can't load texture");
    //    }

    //    ifs.read(reinterpret_cast<char*>(png_file_in_memory.data()),
    //    pos_in_file); if (!ifs.good())
    //    {
    //        throw std::runtime_error("can't load texture");
    //    }

    //    std::vector<std::byte> image;
    //    unsigned long          w = 0;
    //    unsigned long          h = 0;
    //    int error                = decodePNG(image, w, h,
    //    &png_file_in_memory[0],
    //                          png_file_in_memory.size(), false);

    //    // if there's an error, display it
    //    if (error != 0)
    //    {
    //        std::cerr << "error: " << error << std::endl;
    //        throw std::runtime_error("can't load texture");
    //    }
    //    gen_texture_from_pixels(image.data(), w_new, h_new);
}

void texture_gl_es20::gen_texture_from_pixels(const void*  pixels,
                                              const size_t width,
                                              const size_t height)
{
    glGenTextures(1, &tex_handl);
    GL_CHECK()
    glBindTexture(GL_TEXTURE_2D, tex_handl);
    GL_CHECK();

    GLint   mipmap_level = 0;
    GLint   border       = 0;
    GLsizei width_       = static_cast<GLsizei>(width);
    GLsizei height_      = static_cast<GLsizei>(height);
    glTexImage2D(GL_TEXTURE_2D, mipmap_level, GL_RGBA, width_, height_, border,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    GL_CHECK();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GL_CHECK();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GL_CHECK();
}

texture_gl_es20::~texture_gl_es20()
{
    glDeleteTextures(1, &tex_handl);
    GL_CHECK()
}
void texture_gl_es20::bind() const
{
    GLboolean is_texture = glIsTexture(tex_handl);
    SDL_assert(is_texture);
    GL_CHECK()
    glBindTexture(GL_TEXTURE_2D, tex_handl);
    GL_CHECK()
}
