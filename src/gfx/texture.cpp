#include "ff/gfx/texture.hpp"

#include "../external/glew.hpp"
#include "../external/sdl.hpp"
#include "ff/util/log.hpp"

#include <span>

#include <SDL_image.h>

namespace ff {

struct texture_init {
    u32 id;
    uvec2 size;
};

void set_texture_param(u32 pname, texture_wrap wrap_type) {
    switch (wrap_type) {
    case texture_wrap::ClampEdge:
        glCheck(glTexParameteri(GL_TEXTURE_2D, pname, GL_CLAMP_TO_EDGE));
        break;
    case texture_wrap::ClampBorder:
        glCheck(glTexParameteri(GL_TEXTURE_2D, pname, GL_CLAMP_TO_BORDER));
        break;
    case texture_wrap::Mirror:
        glCheck(glTexParameteri(GL_TEXTURE_2D, pname, GL_MIRRORED_REPEAT));
        break;
    case texture_wrap::Repeat:
        glCheck(glTexParameteri(GL_TEXTURE_2D, pname, GL_REPEAT));
        break;
    case texture_wrap::MirrorClampEdge:
        glCheck(glTexParameteri(GL_TEXTURE_2D, pname, GL_MIRROR_CLAMP_TO_EDGE));
        break;
    }
}

void set_texture_param(u32 pname, texture_filter filter_type) {
    switch (filter_type) {
    case texture_filter::Nearest:
        glCheck(glTexParameteri(GL_TEXTURE_2D, pname, GL_NEAREST));
        break;
    case texture_filter::Linear:
        glCheck(glTexParameteri(GL_TEXTURE_2D, pname, GL_LINEAR));
        break;
    }
}

u32 create_texture_from_info(const texture_info& t_format) {
    u32 id;
    glCheck(glGenTextures(1, &id));
    glCheck(glBindTexture(GL_TEXTURE_2D, id));

    set_texture_param(GL_TEXTURE_WRAP_S, t_format.wrap_x);
    set_texture_param(GL_TEXTURE_WRAP_T, t_format.wrap_y);
    set_texture_param(GL_TEXTURE_MIN_FILTER, t_format.filter_min);
    set_texture_param(GL_TEXTURE_MAG_FILTER, t_format.filter_mag);

    return id;
}

std::optional<texture_init> init_color_texture(uvec2 t_size, std::span<color> t_pixels, const texture_info& t_info) {
    u32 id = create_texture_from_info(t_info);
    GLenum mode;
    switch (t_info.format) {
        case texture_format::RGB:  mode = GL_RGB;  break;
        case texture_format::RGBA: mode = GL_RGBA; break;
        default: assert(false);
    }
    glCheck(glTexImage2D(GL_TEXTURE_2D, 0, mode, t_size.y, t_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, t_pixels.data()));
    return texture_init{ id, t_size };
}

texture::texture(std::filesystem::path t_image, texture_info t_info) {
    if (!std::filesystem::exists(t_image)) {
        ff::error("unable to load texture: {} not found", t_image.string());
        return;
    }

    std::string path = t_image.string();
    SDL_Surface* surface = IMG_Load(path.c_str());
    checkSDL(surface);

    SDL_Surface* source = nullptr;
    uvec2 size;
    std::optional<texture_init> init;
    if (surface) {
        source = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        checkSDL(source);
        if (!source) {
            throw std::runtime_error(std::string{"failed to convert image to RGBA: "} + t_image.string());
        }
        else {
            size = { source->w, source->h };
            init = init_color_texture(size, std::span{ (color*)source->pixels, size.x * size.y }, t_info);
            SDL_FreeSurface(source);
        }
        SDL_FreeSurface(surface);
    }

    if (init) {
        m_id   = init->id;
        m_size = init->size;
    }
}

texture::texture(uvec2 t_size, color t_color, texture_info t_info) {

}

texture::texture(texture&& t_texture) {
    *this = std::move(t_texture);
}

texture& texture::operator=(texture&& t_texture) {
    std::swap(m_id,   t_texture.m_id);
    std::swap(m_size, t_texture.m_size);
}

texture::~texture() {
    if (m_id) {
        glCheck(glDeleteTextures(1, &m_id));
    }
}

void texture::bind(u32 t_unit) const {
    glCheck(glBindTexture(GL_TEXTURE0 + t_unit, m_id));
}

}