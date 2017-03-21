#pragma once
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm\mat4x4.hpp>
#include <glm\gtc\quaternion.hpp>
#include "shaders\ShaderFactory.h"
#include "text\SentenceManager.h"
#include "filters\FrequencySpectrum.h"
#include "filters\IQSpectrum.h"
#include "filters\Spectrum.h"
#include "sdr\Sdr.h"
#include "sdr\SdrBuffer.h"
#include "Pane.h"
#include "Viewer.h"
#include "AudioExporter.h"

class Lux
{
    GLFWwindow* window;
    ShaderFactory shaderFactory;
    SentenceManager sentenceManager;
    Viewer viewer;

    Sdr sdr;
    SdrBuffer dataBuffer;

    // Pane-based display items.
    FrequencySpectrum* fourierFilter;
    Pane* fourierTransformPane;

    IQSpectrum* iqSpectrum;
    Pane* iqSpectrumPane;

    Spectrum* spectrum;
    Pane* spectrumPane;

    AudioExporter* audioExporter;
    
    // Top-level display items.
    float fpsTimeAggregated;
    int fpsFramesCounted;
    int fpsSentenceId;
    int dataSpeedSentenceId;
    int mouseToolTipSentenceId;
    void UpdateFps(float frameTime);

    bool LoadCoreGlslGraphics();
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