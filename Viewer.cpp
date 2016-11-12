#include <glm\gtc\matrix_transform.hpp>
#include "Viewer.h"

Viewer::Viewer()
{
    position = glm::vec3(0, 0, -10.0f); // LH coordinate system, Z+ is into the screen.
    target = glm::vec3(0, 0, 0);
    up = glm::vec3(0, 1, 0);
    viewMatrix = glm::lookAtLH(position, target, up);
}