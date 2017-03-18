#include <sstream>
#include <string>
#include <glm\gtx\transform.hpp>
#include <SFML\System.hpp>
#include "logging\Logger.h"
#include "filters\IQSpectrum.h"
#include "filters\FrequencySpectrum.h"
#include "Input.h"
#include "LineRenderer.h"
#include "PointRenderer.h"
#include "version.h"
#include "Lux.h"

#pragma comment(lib, "opengl32")
#pragma comment(lib, "lib/glfw3.lib")
#pragma comment(lib, "lib/glew32.lib")
#pragma comment(lib, "lib/sfml-audio")
#pragma comment(lib, "lib/sfml-system")

Lux::Lux() 
    : shaderFactory(), sentenceManager(), viewer(),
      sdr(), dataBuffer(&sdr, 0, 30), // TODO config somewhere, with device ID passed in somewhere else
      fpsTimeAggregated(0.0f), fpsFramesCounted(0)
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

bool Lux::LoadCoreGlslGraphics()
{
    // 24 depth bits, 8 stencil bits, 8x AA, major version 4.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_STENCIL_BITS, 16);
    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(viewer.ScreenWidth, viewer.ScreenHeight, "Lux", nullptr, nullptr);
    if (!window)
    {
        Logger::LogError("Could not create the GLFW window!");
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    Input::Setup(window, &viewer);

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

    //// Setup any core assets ////
    if (!sentenceManager.LoadFont(&shaderFactory, "fonts/DejaVuSans.ttf"))
    {
        Logger::LogError("Font loading failure!");
        return false;
    }

    if (!PointRenderer::LoadProgram(&shaderFactory))
    {
        Logger::LogError("Could not load the point rendering program!");
        return false;
    }

    if (!LineRenderer::LoadProgram(&shaderFactory))
    {
        Logger::LogError("Could not load the line rendering program!");
        return false;
    }

    return true;
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

    Logger::Log("Setting center frequency: ", sdr.SetCenterFrequency(0, 106100000)); // 452, 734, 0 89,500,0, 106,100,0
    Logger::Log("Setting sampling rate to max w/o dropped packets: ", sdr.SetSampleRate(0, 2400000));
    Logger::Log("Setting bandwidth to sampling rate to use quadrature sampling: ", sdr.SetTunerBandwidth(0, 2400000));
    Logger::Log("Setting auto-gain off: Tuner: ", sdr.SetTunerGainMode(0, true), " Internal: ", sdr.SetInternalAutoGain(0, false));
    Logger::Log("Setting gain of tuner: ", sdr.SetTunerGain(0, 0));
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

void Lux::UpdateFps(float frameTime)
{
    ++fpsFramesCounted;
    fpsTimeAggregated += frameTime;
    if (fpsTimeAggregated > 1.0f)
    {
        std::stringstream framerate;
        framerate << "FPS: " << (float)((float)fpsFramesCounted / fpsTimeAggregated);
        sentenceManager.UpdateSentence(fpsSentenceId, framerate.str());

        fpsTimeAggregated = 0;
        fpsFramesCounted = 0;
    }
}

// TODO hacky code to remove with a redesign. Still prototyping here...
int gainId = 0;
void Lux::Update(float currentTime, float frameTime)
{
    viewer.Update(frameTime);
    
    glm::vec2 screenPos = viewer.GetGridPos(Input::GetMousePos());
    std::stringstream mousePos;
    mousePos << screenPos.x << ", " << screenPos.y;
    sentenceManager.UpdateSentence(mouseToolTipSentenceId, mousePos.str());

    std::stringstream speed;
    speed << "Rate: " << dataBuffer.GetCurrentSampleRate();
    sentenceManager.UpdateSentence(dataSpeedSentenceId, speed.str());

    UpdateFps(frameTime);
    
    if (Input::IsKeyTyped(GLFW_KEY_UP))
    {
        ++gainId;
        gainId = std::min((int)sdr.GetTunerGainSettings(0).size() - 1, gainId);
        Logger::Log("Setting gain of tuner to ID ", gainId, ": ", sdr.SetTunerGain(0, gainId));
    }
    else if (Input::IsKeyTyped(GLFW_KEY_DOWN))
    {
        --gainId;
        gainId = std::max(gainId, 0);
        Logger::Log("Setting gain of tuner to ID ", gainId, ": ", sdr.SetTunerGain(0, gainId));
    }

    // Update our panes.
    fourierTransformPane->Update(currentTime, frameTime);
    iqSpectrumPane->Update(currentTime, frameTime);
    spectrumPane->Update(currentTime, frameTime);
}

void Lux::Render(glm::mat4& viewMatrix)
{
    glm::mat4 projectionMatrix = viewer.perspectiveMatrix * viewMatrix;

    // Clear the screen (and depth buffer) before any rendering begins.
    const GLfloat color[] = { 0, 0, 0, 1 };
    const GLfloat one = 1.0f;
    glClearBufferfv(GL_COLOR, 0, color);
    glClearBufferfv(GL_DEPTH, 0, &one);

    // Render the FPS and our current data transfer rate in the upper-left corner.
    float dataSpeedHeight = sentenceManager.GetSentenceDimensions(dataSpeedSentenceId).y;
    float fpsHeight = sentenceManager.GetSentenceDimensions(fpsSentenceId).y;
    sentenceManager.RenderSentence(fpsSentenceId, viewer.perspectiveMatrix, glm::translate(glm::vec3(-viewer.GetXSize() / 2.0f, viewer.GetYSize() / 2.0f - (dataSpeedHeight + fpsHeight), 0.0f)) * viewMatrix);
    sentenceManager.RenderSentence(dataSpeedSentenceId, viewer.perspectiveMatrix, glm::translate(glm::vec3(-viewer.GetXSize() / 2.0f, viewer.GetYSize() / 2.0f - dataSpeedHeight, 0.0f)) * viewMatrix);

    // Render our mouse tool tip ... at the mouse
    glm::vec2 screenPos = viewer.GetGridPos(Input::GetMousePos());
    sentenceManager.RenderSentence(mouseToolTipSentenceId, viewer.perspectiveMatrix, glm::translate(glm::vec3(screenPos.x, screenPos.y, 0.0f)) * viewMatrix);

    // Render all our filters.
    // iqFilter->Render(projectionMatrix); // render the IQ one only.
    // for (FilterBase* filter : filters)
    // {
    //     filter->Render(projectionMatrix);
    // }

    // Render the panes
    fourierTransformPane->Render(projectionMatrix, viewer.perspectiveMatrix, viewMatrix);
    iqSpectrumPane->Render(projectionMatrix, viewer.perspectiveMatrix, viewMatrix);
    spectrumPane->Render(projectionMatrix, viewer.perspectiveMatrix, viewMatrix);
}

bool Lux::LoadGraphics()
{
    if (!LoadCoreGlslGraphics())
    {
        return false;
    }

    // TODO test code remove
    fpsSentenceId = sentenceManager.CreateNewSentence();
    sentenceManager.UpdateSentence(fpsSentenceId, "FPS:", 14, glm::vec3(1.0f, 1.0f, 1.0f));

    dataSpeedSentenceId = sentenceManager.CreateNewSentence();
    sentenceManager.UpdateSentence(dataSpeedSentenceId, "Speed: ", 14, glm::vec3(1.0f, 1.0f, 1.0f));

    mouseToolTipSentenceId = sentenceManager.CreateNewSentence();
    sentenceManager.UpdateSentence(mouseToolTipSentenceId, "(?,?)", 12, glm::vec3(1.0f, 1.0f, 1.0f));

    glm::vec2 panePos = glm::vec2(-60.0f, -30.0f);
    glm::vec2 paneSize = glm::vec2(30.0f, 30.0f);
    fourierFilter = new FrequencySpectrum(panePos, paneSize, &dataBuffer);
    fourierTransformPane = new Pane(panePos, paneSize, &viewer, &sentenceManager, fourierFilter);

    panePos = glm::vec2(-29.0f, -30.0f);
    iqSpectrum = new IQSpectrum(panePos, paneSize, &dataBuffer);
    iqSpectrumPane = new Pane(panePos, paneSize, &viewer, &sentenceManager, iqSpectrum);

    panePos = glm::vec2(2.0f, -30.0f);
    spectrum = new Spectrum(panePos, paneSize, &dataBuffer);
    spectrumPane = new Pane(panePos, paneSize, &viewer, &sentenceManager, spectrum);

    return true;
}

void Lux::UnloadGraphics()
{
    delete fourierTransformPane;
    delete fourierFilter;

    delete iqSpectrumPane;
    delete iqSpectrum;

    delete spectrumPane;
    delete spectrum;

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
        sf::Int64 sleepDelay = ((long)1e6 / viewer.MaxFramerate) - (long)(frameTime * 1e6);
        if (sleepDelay > 0)
        {
            sf::sleep(sf::microseconds(sleepDelay));
        }
    }

    return true;
}

int main()
{
    Logger::Setup("lux-log.log", true);
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