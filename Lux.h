#pragma once
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm\mat4x4.hpp>
#include <glm\gtc\quaternion.hpp>
#include "shaders\ShaderFactory.h"
#include "text\SentenceManager.h"
#include "sdr\Sdr.h"
#include "sdr\SdrBuffer.h"
#include "Viewer.h"

class Lux
{
    GLFWwindow* window;
    ShaderFactory shaderFactory;
    SentenceManager sentenceManager;
    Viewer viewer;

    Sdr sdr;
    SdrBuffer dataBuffer;

    // TODO test code remove.
    int sentenceId;
    int dataSpeedSentenceId;

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