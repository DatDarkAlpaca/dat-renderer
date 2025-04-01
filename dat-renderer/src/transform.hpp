#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct transform
{
public:
    inline glm::mat4 get_transform() const
    {
        glm::mat4 transform(1.0f);
        transform = glm::translate(transform, position);
        transform = glm::scale(transform, scale);

        return transform;
    }

public:
    glm::vec3 position;
    // rotation
    glm::vec3 scale;
};