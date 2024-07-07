#include "ff/gfx/texture.hpp"

#include "../external/glew.hpp"
#include "../external/sdl.hpp"
#include "ff/util/log.hpp"

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

texture_init init_from_surface(SDL_Surface* t_surface, const texture_info& t_format) {

    u32 id;
    glCheck(glGenTextures(1, &id));
    glCheck(glBindTexture(GL_TEXTURE_2D, id));

    set_texture_param(GL_TEXTURE_WRAP_S, t_format.wrap_x);
    set_texture_param(GL_TEXTURE_WRAP_T, t_format.wrap_y);
    set_texture_param(GL_TEXTURE_MIN_FILTER, t_format.filter_min);
    set_texture_param(GL_TEXTURE_MAG_FILTER, t_format.filter_mag);

    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    // determine SDL surface image format
    GLenum mode;
    switch (t_format.format) {
    case texture_format::RGB:          mode = GL_RGB;             break;
    case texture_format::RGBA:         mode = GL_RGBA;            break;
    case texture_format::Depth:        mode = GL_DEPTH_COMPONENT; break;
    case texture_format::DepthStencil: mode = GL_DEPTH_STENCIL;   break;
    }
    SDL_Surface* conv_surface = SDL_ConvertSurfaceFormat(t_surface, SDL_PIXELFORMAT_RGBA32, 0);
    checkSDL(conv_surface);

    glCheck(glTexImage2D(GL_TEXTURE_2D, 0, mode, t_surface->w, t_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, conv_surface->pixels));

    texture_init tex;
    tex.id   = id;
    tex.size = { t_surface->w, t_surface->h };
    SDL_FreeSurface(conv_surface);
    return tex;
}

texture::texture(std::filesystem::path t_image, texture_info t_info) {
    if (!std::filesystem::exists(t_image)) {
        ff::error("unable to load texture: {} not found", t_image.string());
        return;
    }

    SDL_Surface* surface = IMG_Load(t_image.c_str());
    checkSDL(surface);

    auto init_info = init_from_surface(surface, t_info);
    SDL_FreeSurface(surface);
}

texture::texture(uvec2 t_size, texture_info t_info) {

}

texture::texture(texture&& t_texture) {

}

texture& texture::operator=(texture&& t_texture) {

}

texture::~texture() {

}

void texture::bind(u32 t_unit) const {

}

}