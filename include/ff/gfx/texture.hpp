#pragma once

#include "ff/util/math.hpp"
#include "ff/gfx/color.hpp"

#include <filesystem>

namespace ff {

enum class texture_filter {
    Nearest,
    Linear
};

enum class texture_wrap {
    ClampEdge,
    ClampBorder,
    Mirror,
    Repeat,
    MirrorClampEdge
};

enum class texture_format {
    RGB,
    RGBA,
    Depth32,
    Depth24Stencil8
};

struct texture_info {
    texture_filter filter_mag = texture_filter::Nearest;
    texture_filter filter_min = texture_filter::Linear;
    texture_wrap   wrap_x     = texture_wrap::ClampEdge;
    texture_wrap   wrap_y     = texture_wrap::ClampEdge;
    texture_format format     = texture_format::RGBA;
};

class texture {
public:
    texture(std::filesystem::path t_image, texture_info t_info = {});
    texture(uvec2 t_size, u32 value, texture_info t_info = {});

    texture(const texture&) = delete;
    texture& operator=(const texture&) = delete;
    texture(texture&&);
    texture& operator=(texture&&);
    ~texture();

    inline u32 id() const { return m_id; };
    [[nodiscard]] inline uvec2 size() const { return m_size; };

    void bind(u32 t_unit = 0) const;

private:
    u32 m_id;
    uvec2 m_size;
};

}