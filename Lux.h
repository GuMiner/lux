#pragma once
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm\mat4x4.hpp>
#include <glm\gtc\quaternion.hpp>
#include "factories\ShaderFactory.h"

struct GlobalGraphicsSettings
{
    glm::vec3 viewerPosition;
    glm::quat viewerOrientation;

    bool isPolygonFillMode;

    GlobalGraphicsSettings()
        : isPolygonFillMode(true), viewerPosition(0.0f), viewerOrientation()
    {
    }
};

class Lux
{
    GLFWwindow* window;
    GlobalGraphicsSettings settings;
    ShaderFactory shaderFactory;

    void LogSystemSetup();

    void HandleEvents(bool& focusPaused, bool& escapePaused);
    void Update(float currentTime, float frameTime);
    void Render(glm::mat4& viewMatrix);

public:
    Lux();

    bool Initialize();
    void Deinitialize();

    bool LoadGraphics();
    void UnloadGraphics();
    
    bool Run();
};