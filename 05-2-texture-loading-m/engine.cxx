#include "engine.hxx"
#include "SDL.h"
#include "glad/glad.h"
#include "picopng.hxx"

#include <cassert>
#include <exception>
#include <iostream>
#include <sstream>
#include <vector>
#define GL_CHECK()                                                             \
    {                                                                          \
        const int err = static_cast<int>(glGetError());                        \
        if (err != GL_NO_ERROR)                                                \
        {                                                                      \
            switch (err)                                                       \
            {                                                                  \
                case GL_INVALID_ENUM:                                          \
                    std::cerr << GL_INVALID_ENUM << std::endl;                 \
                    break;                                                     \
                case GL_INVALID_VALUE:                                         \
                    std::cerr << GL_INVALID_VALUE << std::endl;                \
                    break;                                                     \
                case GL_INVALID_OPERATION:                                     \
                    std::cerr << GL_INVALID_OPERATION << std::endl;            \
                    break;                                                     \
                case GL_INVALID_FRAMEBUFFER_OPERATION:                         \
                    std::cerr << GL_INVALID_FRAMEBUFFER_OPERATION              \
                              << std::endl;                                    \
                    break;                                                     \
                case GL_OUT_OF_MEMORY:                                         \
                    std::cerr << GL_OUT_OF_MEMORY << std::endl;                \
                    break;                                                     \
            }                                                                  \
            assert(false);                                                     \
        }                                                                      \
    }

template <typename T>
static void load_gl_func(const char* func_name, T& result)
{
    void* gl_pointer = SDL_GL_GetProcAddress(func_name);
    if (nullptr == gl_pointer)
    {
        throw std::runtime_error(std::string("can't load GL function") +
                                 func_name);
    }
    result = reinterpret_cast<T>(gl_pointer);
}

std::ostream& operator<<(std::ostream& os, const SDL_version& v)
{
    os << v.major << ':';
    os << v.minor << ':';
    os << v.patch;
    return os;
}

std::istream& operator>>(std::istream& is, vertex& v)
{
    is >> v.x;
    is >> v.y;
    is >> v.tx;
    is >> v.ty;

    return is;
}
std::istream& operator>>(std::istream& is, triangle& t)
{
    is >> t.v[0];
    is >> t.v[1];
    is >> t.v[2];
    return is;
}

class engine_impl final : public engine
{
public:
    std::string initialize(std::string_view str) override final
    {

        using namespace std;

        stringstream serr;

        SDL_version compiled = { 0, 0, 0 };
        SDL_version linked   = { 0, 0, 0 };
        SDL_VERSION(&compiled);
        SDL_GetVersion(&linked);
        if (SDL_COMPILEDVERSION !=
            SDL_VERSIONNUM(linked.major, linked.minor, linked.patch))
        {
            serr << "warning: SDL2 compiled and linked version mismatch: "
                 << compiled << " " << linked << endl;
        }

        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        {
            serr << "error: failed call SDL_Init: " << SDL_GetError() << endl;
            return serr.str();
        }

        window = SDL_CreateWindow("window 05-2", SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED, 640, 480,
                                  ::SDL_WINDOW_OPENGL);
        if (window == nullptr)
        {
            serr << "error: failed call SDL_CreateWindow: " << SDL_GetError()
                 << endl;
            return serr.str();
        }

        gl_context = SDL_GL_CreateContext(window);
        if (gl_context == nullptr)
        {
            std::string msg("can't create opengl context: ");
            msg += SDL_GetError();
            serr << msg << endl;
            return serr.str();
        }

        int gl_major_ver = 2;

        int result =
            SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_major_ver);
        SDL_assert(result == 0);
        int gl_minor_ver = 0;
        result =
            SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &gl_minor_ver);
        SDL_assert(result == 0);

        if (gl_major_ver <= 2 && gl_minor_ver < 1)
        {
            serr << "current context opengl version: " << gl_major_ver << '.'
                 << gl_minor_ver << '\n'
                 << "need opengl version at least: 2.1\n"
                 << std::flush;
            return serr.str();
        }

        if (gladLoadGLES2Loader(SDL_GL_GetProcAddress) == 0)
        {
            std::cerr << "error: failed to initialize glad" << std::endl;
        }

        // create vertex shader
        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        GL_CHECK();
        string_view vertex_source = R"(
                                            attribute vec2 a_position;
                                            attribute vec2 a_tex_coord;
                                            varying vec2 v_tex_coord;
                                            void main()
                                            {
                                                v_tex_coord = -1.f*a_tex_coord*3.f;
                                                gl_Position = vec4(a_position*2.f, 0.0, 1.0);
                                            }
                                            )";
        const char* source        = vertex_source.data();
        glShaderSource(vertex_shader, 1, &source, nullptr);
        GL_CHECK();

        glCompileShader(vertex_shader);
        GL_CHECK();

        GLint compiled_status = 0;
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled_status);
        GL_CHECK()
        if (compiled_status == 0)
        {
            GLint info_len = 0;
            glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &info_len);
            GL_CHECK()
            std::vector<char> info_chars(static_cast<size_t>(info_len));
            glGetShaderInfoLog(vertex_shader, info_len, nullptr,
                               info_chars.data());
            GL_CHECK()
            glDeleteShader(vertex_shader);
            GL_CHECK()

            serr << "Error compiling shader(vertex)\n"
                 << vertex_source << "\n"
                 << info_chars.data();
            return serr.str();
        }

        // create fragment shader

        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        GL_CHECK();
        string_view fragment_source = R"(
                                                varying vec2 v_tex_coord;
                                                uniform sampler2D s_texture;
                                                void main()
                                                {
                                                    gl_FragColor = texture2D(s_texture, v_tex_coord);
                                                }
                                                )";
        source                      = fragment_source.data();
        glShaderSource(fragment_shader, 1, &source, nullptr);
        GL_CHECK();

        glCompileShader(fragment_shader);
        GL_CHECK();

        compiled_status = 0;
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled_status);
        GL_CHECK()
        if (compiled_status == 0)
        {
            GLint info_len = 0;
            glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &info_len);
            GL_CHECK()
            std::vector<char> info_chars(static_cast<size_t>(info_len));
            glGetShaderInfoLog(fragment_shader, info_len, nullptr,
                               info_chars.data());
            GL_CHECK()
            glDeleteShader(fragment_shader);
            GL_CHECK()

            serr << "Error compiling shader(fragment)\n"
                 << fragment_source << "\n"
                 << info_chars.data();
            return serr.str();
        }

        // create program and attach vertex and fragment shaders

        GLuint program_id_ = glCreateProgram();
        GL_CHECK()
        if (0 == program_id_)
        {
            cerr << "error: can't create GL_program";
            return serr.str();
        }

        glAttachShader(program_id_, vertex_shader);
        GL_CHECK()
        glAttachShader(program_id_, fragment_shader);
        GL_CHECK()

        glBindAttribLocation(program_id_, 0, "a_position");
        GL_CHECK()

        glLinkProgram(program_id_);
        GL_CHECK()

        GLint linked_status = 0;
        glGetProgramiv(program_id_, GL_LINK_STATUS, &linked_status);
        GL_CHECK()
        if (linked_status == 0)
        {
            GLint infoLen = 0;
            glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &infoLen);
            GL_CHECK()
            std::vector<char> infoLog(static_cast<size_t>(infoLen));
            glGetProgramInfoLog(program_id_, infoLen, nullptr, infoLog.data());
            GL_CHECK()
            serr << "Error linking program:\n" << infoLog.data();
            glDeleteProgram(program_id_);
            GL_CHECK()
            return serr.str();
        }
        glUseProgram(program_id_);
        GL_CHECK()

        /// add texture
        int location = glGetUniformLocation(program_id_, "s_texture");
        GL_CHECK();
        assert(-1 != location);
        int texture_unit = 0;
        glActiveTexture(GL_TEXTURE0 + texture_unit);
        GL_CHECK();

        if (!load_texture("tank.png"))
        {
            return "failed load texture\n";
        }

        // http://www.khronos.org/opengles/sdk/docs/man/xhtml/glUniform.xml
        glUniform1i(location, 0 + texture_unit);
        GL_CHECK();

        glEnable(GL_BLEND);
        GL_CHECK();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        GL_CHECK();

        return "";
    }
    void uninitialize() override final
    {
        // glDeleteProgram(program_id_);
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    void render_triangle(const triangle& t) override final
    {
        // vertex
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
                              &t.v[0].x);
        GL_CHECK()
        // first attribute - vertex
        glEnableVertexAttribArray(0);
        GL_CHECK()
        // texture
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
                              &t.v[0].tx);
        GL_CHECK();
        glEnableVertexAttribArray(1);
        GL_CHECK();

        //        glValidateProgram(program_id_);
        //        GL_CHECK()
        //        // Check the validate status
        //        GLint validate_status = 0;
        //        glGetProgramiv(program_id_, GL_VALIDATE_STATUS,
        //        &validate_status); GL_CHECK() if (validate_status == GL_FALSE)
        //        {
        //            GLint infoLen = 0;
        //            glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &infoLen);
        //            GL_CHECK()
        //            std::vector<char> infoLog(static_cast<size_t>(infoLen));
        //            glGetProgramInfoLog(program_id_, infoLen, nullptr,
        //            infoLog.data()); GL_CHECK() std::cerr << "Error linking
        //            program :\n " << infoLog.data(); throw
        //            std::runtime_error("error");
        //        }
        glDrawArrays(GL_TRIANGLES, 0, 3);
        GL_CHECK()
    }

    void swap_buffer() override final
    {
        SDL_GL_SwapWindow(window);
        glClearColor(0.3f, 0.3f, 1.0f, 0.0f);
        GL_CHECK()
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_CHECK()
    }

    bool read_input(bool& key) override final
    {
        using namespace std;
        SDL_Event sdl_event;
        if (SDL_PollEvent(&sdl_event))
        {
            if (sdl_event.type == SDL_QUIT)
            {
                key = false;
                return true;
            }
        }

        return false;
    }

    float get_time_from_init() override final
    {
        std::uint32_t time_from_begining = SDL_GetTicks();
        float         current_time       = time_from_begining * 0.001f;
        return current_time;
    }

    bool load_texture(std::string_view path) override final
    {
        std::vector<unsigned char> png_file_in_memory;
        std::ifstream              ifs(path.data(), std::ios_base::binary);
        if (!ifs)
        {
            return false;
        }
        ifs.seekg(0, std::ios_base::end);
        size_t pos_in_file = static_cast<size_t>(ifs.tellg());
        png_file_in_memory.resize(pos_in_file);
        ifs.seekg(0, std::ios_base::beg);
        if (!ifs)
        {
            return false;
        }

        ifs.read(reinterpret_cast<char*>(png_file_in_memory.data()),
                 pos_in_file);
        if (!ifs.good())
        {
            return false;
        }

        std::vector<unsigned char> image;
        unsigned long              w = 0;
        unsigned long              h = 0;
        int error = decodePNG(image, w, h, &png_file_in_memory[0],
                              png_file_in_memory.size(), false);

        // if there's an error, display it
        if (error != 0)
        {
            std::cerr << "error: " << error << std::endl;
            return false;
        }
        // индентификатор текстуры
        GLuint tex_handl = 0;
        //Первый указывает количество имен-идентификаторов текстур,
        //которые нужно создать. Второй параметр - указатель на массив элементов
        //типа GLuint.
        glGenTextures(1, &tex_handl);
        GL_CHECK()
        //привязываемся к текстуре фотографии, т.е. делаем ее активной.
        glBindTexture(GL_TEXTURE_2D, tex_handl);
        GL_CHECK()

        GLint mipmap_level = 0;
        GLint border       = 0;
        //создать саму текстуру в памяти
        // clang-format off
        glTexImage2D(GL_TEXTURE_2D, //Specifies the target texture of the active texture unit.
                     mipmap_level,  //Specifies the level-of-detail number. Level 0 is the base image level
                     GL_RGBA,       //Specifies the internal format of the texture.
                     static_cast<GLsizei> (w),
                     static_cast<GLsizei> (h),
                     border,        //Specifies the width of the border. Must be 0
                     GL_RGBA,       //Specifies the format of the texel data. Must match internalformat
                     GL_UNSIGNED_BYTE, //Specifies the data type of the texel data. rgba - (0,255)
                     &image[0]);    //pointer on array
        // clang-format on
        GL_CHECK()
        //установить параметры текстуры
        // for _MIN_FILTER (min picture) - GL_NEAREST
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        GL_CHECK()
        // for _MAG_FILTER (big picture)  - GL_NEAREST
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        GL_CHECK()
        return true;
    }

private:
    SDL_Window*   window     = nullptr;
    SDL_GLContext gl_context = nullptr;
    // GLuint        program_id_ = 0;
};
engine::~engine() {}

static bool already_exist = false;

engine* create_engine()
{
    if (true == already_exist)
    {
        throw std::runtime_error("engine allredy exist");
    }
    engine* eng   = new engine_impl();
    already_exist = true;
    return eng;
}
void destroy_engine(engine* eng)
{
    if (false == already_exist)
    {
        throw std::runtime_error("engine no exist");
    }
    if (nullptr == eng)
    {
        throw std::runtime_error("engine is null");
    }
    delete eng;
}
