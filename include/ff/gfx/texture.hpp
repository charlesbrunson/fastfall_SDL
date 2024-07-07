#pragma once

#include "ff/util/math.hpp"

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
    Depth,
    DepthStencil
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
    texture(uvec2 t_size, texture_info t_info = {});

    texture(const texture&) = delete;
    texture& operator=(const texture&) = delete;
    texture(texture&&);
    texture& operator=(texture&&);
    ~texture();

    void bind(u32 t_unit = 0) const;
    [[nodiscard]] uvec2 size() const;

private:
    u32 m_id;
    uvec2 m_size;
};

}