#include <sstream>
#include <string>
#include <glm\gtc\matrix_transform.hpp>
#include <SFML\System.hpp>
#include "logging\Logger.h"
#include "IQSpectrum.h"
#include "Input.h"
#include "GraphicsSetup.h"
#include "Hertz.h"
#include "version.h"
#include "Lux.h"

#pragma comment(lib, "opengl32")
#pragma comment(lib, "lib/glfw3.lib")
#pragma comment(lib, "lib/glew32.lib")
#pragma comment(lib, "lib/sfml-audio")
#pragma comment(lib, "lib/sfml-system")

Lux::Lux() 
    : shaderFactory(), sentenceManager(), viewer(),
      sdr(), dataBuffer(&sdr, 0, 30), filters() // TODO configuration somewhere. TODO device ID should be passed-in separately.
{
}

void Lux::LogSystemSetup()
{
    Logger::Log("OpenGL vendor: ", glGetString(GL_VENDOR), ", version ", glGetString(GL_VERSION), ", renderer ", glGetString(GL_RENDERER));
    Logger::Log("OpenGL extensions: ", glGetString(GL_EXTENSIONS));

    GLint maxTextureUnits, maxUniformBlockSize;
    GLint maxVertexUniformBlocks, maxFragmentUniformBlocks;
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &maxVertexUniformBlocks);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &maxFragmentUniformBlocks);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

    Logger::Log("Max Texture Units: ", maxTextureUnits, ", Max Uniform Size: ", (maxUniformBlockSize / 1024), " kB");
    Logger::Log("Max Vertex Uniform Blocks: ", maxVertexUniformBlocks, ", Max Fragment Uniform Blocks: ", maxFragmentUniformBlocks);
    Logger::Log("Max Texture Size: ", maxTextureSize);
}

bool Lux::Initialize()
{
    if (!sdr.Initialize())
    {
        Logger::LogError("SDR startup failure!");
        return false;
    }

    // Note we don't need to remove the device as deletion will handle that for us.
    Logger::Log("Open device: ", sdr.OpenDevice(0));

    Logger::Log("Setting center frequency: ", sdr.SetCenterFrequency(0, Hertz(452,734,0).GetFrequency())); // 452, 734, 0 89,500,0
    Logger::Log("Setting sampling rate to max w/o dropped packets: ", sdr.SetSampleRate(0, Hertz(2, 400, 0).GetFrequency()));
    Logger::Log("Setting bandwidth to sampling rate to use quadrature sampling: ", sdr.SetTunerBandwidth(0, Hertz(2, 400, 0).GetFrequency()));
    Logger::Log("Setting auto-gain: Tuner: ", sdr.SetTunerGainMode(0, false), " Internal: ", sdr.SetInternalAutoGain(0, true));
    dataBuffer.StartAcquisition();

    // Setup GLFW
    if (!glfwInit())
    {
        Logger::LogError("GLFW startup failure");
        return false;
    }

    return true;
}

void Lux::Deinitialize()
{
    dataBuffer.StopAcquisition();

    glfwTerminate();
}

void Lux::HandleEvents(bool& focusPaused, bool& escapePaused)
{
    glfwPollEvents();
    focusPaused = !Input::hasFocus;
    escapePaused = Input::IsKeyTyped(GLFW_KEY_ESCAPE);
}

void Lux::Update(float currentTime, float frameTime)
{
    std::stringstream speed;
    speed << "Rate: " << dataBuffer.GetCurrentSampleRate();
    sentenceManager.UpdateSentence(dataSpeedSentenceId, speed.str(), 22, glm::vec3(1.0f, 1.0f, 1.0f));
}

void Lux::Render(glm::mat4& viewMatrix)
{
    glm::mat4 projectionMatrix = GraphicsSetup::PerspectiveMatrix * viewMatrix;

    // Clear the screen (and depth buffer) before any rendering begins.
    const GLfloat color[] = { 0, 0, 0, 1 };
    const GLfloat one = 1.0f;
    glClearBufferfv(GL_COLOR, 0, color);
    glClearBufferfv(GL_DEPTH, 0, &one);

    // TODO render something interesting.
    glm::mat4 sentenceTestMatrix = glm::scale(glm::translate(glm::mat4(), glm::vec3(-0.821f, -0.121f, -1.0f)), glm::vec3(0.022f, 0.022f, 0.02f));
    sentenceManager.RenderSentence(sentenceId, GraphicsSetup::PerspectiveMatrix, sentenceTestMatrix);

    sentenceTestMatrix = glm::scale(glm::translate(glm::mat4(), glm::vec3(-0.821f, -0.091f, -1.0f)), glm::vec3(0.022f, 0.022f, 0.02f));
    sentenceManager.RenderSentence(dataSpeedSentenceId, GraphicsSetup::PerspectiveMatrix, sentenceTestMatrix);

    // Render all our filters.
    for (FilterBase* filter : filters)
    {
        filter->Render(projectionMatrix);
    }
}

bool Lux::LoadGraphics()
{
    // 24 depth bits, 8 stencil bits, 8x AA, major version 4.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_STENCIL_BITS, 16);
    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(GraphicsSetup::ScreenWidth, GraphicsSetup::ScreenHeight, "Lux", nullptr, nullptr);
    if (!window)
    {
        Logger::LogError("Could not create the GLFW window!");
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    Input::Setup(window, GraphicsSetup::ScreenWidth, GraphicsSetup::ScreenHeight);

    // Setup GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        Logger::LogError("GLEW startup failure: ", err, ".");
        return false;
    }

    // Log graphics information for future reference
    LogSystemSetup();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable line, but not polygon smoothing.
    glEnable(GL_LINE_SMOOTH);

    // Let OpenGL shaders determine point sizes.
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Disable face culling so that see-through flat objects and stuff at 1.0 (cube map, text) work.
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CW);

    // Cutout faces that are hidden by other faces.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //// Setup any assets ////
    if (!sentenceManager.LoadFont(&shaderFactory, "fonts/DejaVuSans.ttf"))
    {
        Logger::LogError("Font loading failure!");
        return false;
    }

    // TODO test code remove
    sentenceId = sentenceManager.CreateNewSentence();
    sentenceManager.UpdateSentence(sentenceId, "Test string 123456789-10!", 22, glm::vec3(1.0f, 1.0f, 1.0f));

    dataSpeedSentenceId = sentenceManager.CreateNewSentence();
    sentenceManager.UpdateSentence(dataSpeedSentenceId, "Speed: ", 22, glm::vec3(1.0f, 1.0f, 1.0f));

    filters.push_back(new IQSpectrum(&dataBuffer));
    for (FilterBase* filter : filters)
    {
        if (!filter->LoadGraphics(&shaderFactory))
        {
            Logger::LogError("Error loading filter '", filter->GetName(), "'.");
            return false;
        }
    }

    return true;
}

void Lux::UnloadGraphics()
{
    for (FilterBase* filter : filters)
    {
        delete filter;
    }

    glfwDestroyWindow(window);
    window = nullptr;
}

bool Lux::Run()
{
    sf::Clock clock;
    sf::Clock frameClock;
    sf::Time clockStartTime;
    bool focusPaused = false;
    bool escapePaused = false;
    while (!glfwWindowShouldClose(window))
    {
        clockStartTime = clock.getElapsedTime();

        float frameTime = std::min(frameClock.restart().asSeconds(), 0.06f);
        HandleEvents(focusPaused, escapePaused);

        // Run the game and render if not paused.
        if (!focusPaused && !escapePaused)
        {
            float gameTime = clock.getElapsedTime().asSeconds();
            Update(gameTime, frameTime);

            Render(viewer.viewMatrix);
            glfwSwapBuffers(window);
        }

        // Delay to run approximately at our maximum framerate.
        sf::Int64 sleepDelay = ((long)1e6 / GraphicsSetup::MaxFramerate) - (long)(frameTime * 1e6);
        if (sleepDelay > 0)
        {
            sf::sleep(sf::microseconds(sleepDelay));
        }
    }

    return true;
}

int main()
{
    Logger::Setup("lux-log.log");
    Logger::Log("Lux ", AutoVersion::MAJOR_VERSION, ".", AutoVersion::MINOR_VERSION);

    Lux* lux = new Lux();
    if (!lux->Initialize())
    {
        Logger::LogError("Lux initialization failed!");
    }
    else
    {
        Logger::Log("Basic Initialization complete!");
        if (!lux->LoadGraphics())
        {
            Logger::LogError("Lux graphics initialization failed!");
        }
        else
        {
            Logger::Log("Graphics Initialized!");
            if (!lux->Run())
            {
                Logger::LogError("Lux operation failed!");
            }

            lux->UnloadGraphics();
        }

        lux->Deinitialize();
    }

    delete lux;
    Logger::Log("Done.");
    Logger::Shutdown();
    return 0;
}