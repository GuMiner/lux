#pragma once
#include <string>
#include <glm\mat4x4.hpp>
#include <glm\vec2.hpp>

class IPaneRenderable
{
public:
    // Returns true if the title has changed, false otherwise.
    virtual bool HasTitleUpdate() = 0;

    // Returns the current title of the pane.
    virtual std::string GetTitle() = 0;

    // Updates the contents of the pane based on graphical time-based updates.
    virtual void Update(float elapsedTime, float frameTime) = 0;

    // Renders into the provided position and size on the XY plane.
    virtual void Render(glm::mat4& projectionMatrix, glm::vec2 position, glm::vec2 size) = 0;
};

