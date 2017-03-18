#pragma once
#include <glm\mat4x4.hpp>
#include <glm\vec2.hpp>
#include "IPaneRenderable.h"

class Pane
{
    float unitsPerPixel;

    glm::vec2 position;
    glm::vec2 size;

    IPaneRenderable* paneContents;

public:
    Pane(glm::vec2 position, glm::vec2 size, float unitsPerPixel);

    void Update(float elapsedTime, float frameTime);
    void Render(glm::mat4& projectionMatrix);
};

