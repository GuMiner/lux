#include <glm\gtc\matrix_transform.hpp>
#include "logging\Logger.h"
#include "Input.h"
#include "GraphicsSetup.h"
#include "version.h"
#include "Lux.h"

#pragma comment(lib, "opengl32")
#pragma comment(lib, "lib/glfw3.lib")
#pragma comment(lib, "lib/glew32.lib")
#pragma comment(lib, "lib/sfml-audio")
#pragma comment(lib, "lib/sfml-system")

Lux::Lux() 
    : settings(), shaderFactory()
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
    if (Input::IsKeyTyped(GLFW_KEY_G))
    {
        if (settings.isPolygonFillMode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        settings.isPolygonFillMode = !settings.isPolygonFillMode;
    }
}



void Lux::Render(glm::mat4& viewMatrix)
{
    glm::mat4 projectionMatrix = GraphicsSetup::PerspectiveMatrix * viewMatrix;

    // Clear the screen (and depth buffer) before any rendering begins.
    const GLfloat color[] = { 0, 1, 0, 1 };
    const GLfloat one = 1.0f;
    glClearBufferfv(GL_COLOR, 0, color);
    glClearBufferfv(GL_DEPTH, 0, &one);

    // TODO render something interesting.
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

    window = glfwCreateWindow(GraphicsSetup::ScreenWidth, GraphicsSetup::ScreenHeight, "Advanced Graphics-Open World", nullptr, nullptr);
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

    return true;
}

void Lux::UnloadGraphics()
{
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
    glm::mat4 viewMatrix;
    while (!glfwWindowShouldClose(window))
    {
        clockStartTime = clock.getElapsedTime();
        viewMatrix = glm::mat4_cast(settings.viewerOrientation) * glm::translate(glm::mat4(), -settings.viewerPosition);

        float frameTime = std::min(frameClock.restart().asSeconds(), 0.06f);
        HandleEvents(focusPaused, escapePaused);

        // Run the game and render if not paused.
        if (!focusPaused && !escapePaused)
        {
            float gameTime = clock.getElapsedTime().asSeconds();
            Update(gameTime, frameTime);

            Render(viewMatrix);
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
    Logger::Setup();
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

