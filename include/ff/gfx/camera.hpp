#pragma once

#include <glm/vec2.hpp>
#include <glm/mat3x3.hpp>

#include "ff/util/rect.hpp"

namespace ff {

class camera {
public:
    camera();
    camera(glm::vec2 t_botleft, glm::vec2 t_size, glm::vec2 t_scale = {1.f, 1.f});

    void set_center(glm::vec2 t_center);
    void set_viewport(rectf t_viewport);
    void set_size(glm::vec2 t_size);
    void set_zoom(float t_zoom);

    glm::vec2 get_center() const;
    rectf get_viewport() const;
    glm::vec2 get_size() const;
    float get_zoom() const;

    glm::mat3 matrix() const;
    glm::mat3 inv_matrix() const;

private:
    glm::vec2 m_center;
    rectf m_viewport;
    glm::vec2 m_size;
    float m_zoom = 1.f;
    glm::vec2 m_scale;
};

}
