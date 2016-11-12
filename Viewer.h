#pragma once
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>

class Viewer
{
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
public:
    glm::mat4 viewMatrix;

    Viewer();

    // TODO add view motion functionality.
};

