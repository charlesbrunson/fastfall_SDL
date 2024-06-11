#pragma once

#include <glm/vec2.hpp>
#include <glm/mat3x3.hpp>

#include "ff/util/rect.hpp"

namespace ff {

class camera {
public:
    camera();
    camera(glm::vec2 botleft, glm::vec2 size, glm::vec2 scale = {1.f, 1.f});

    void set_center(glm::vec2 center);
    void set_viewport(rectf viewport);
    void set_size(glm::vec2 size);
    void set_zoom(float zoom);

    glm::vec2 get_center() const;
    rectf get_viewport() const;
    glm::vec2 get_size() const;
    float get_zoom() const;

    glm::mat3 matrix() const;
    glm::mat3 inv_matrix() const;

private:
    glm::vec2 center;
    rectf viewport;
    glm::vec2 size;
    float zoom = 1.f;
    glm::vec2 scale;
};

}
