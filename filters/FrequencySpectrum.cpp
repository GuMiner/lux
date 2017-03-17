#include <algorithm>
#include <chrono>
#include <limits>
#include "FrequencySpectrum.h"
#include "math\FourierTransform.h"

FrequencySpectrum::FrequencySpectrum(SdrBuffer* dataBuffer)
    : firstUpdate(true), FilterBase(dataBuffer)
{
}

bool FrequencySpectrum::LoadGraphics(ShaderFactory* shaderFactory)
{
    Logger::Log("Loading the point rendering shading program...");
    if (!shaderFactory->CreateShaderProgram("pointRender", &spectrumProgram.programId))
    {
        Logger::LogError("Failed to load the point rendering shader; cannot continue.");
        return false;
    }

    spectrumProgram.projMatrixLocation = glGetUniformLocation(spectrumProgram.programId, "projMatrix");
    spectrumProgram.pointSizeLocation = glGetUniformLocation(spectrumProgram.programId, "pointSize");
    spectrumProgram.transparencyLocation = glGetUniformLocation(spectrumProgram.programId, "transparency");

    glGenVertexArrays(1, &spectrumData.vao);
    glBindVertexArray(spectrumData.vao);

    spectrumData.positionBuffer.Initialize();
    spectrumData.colorBuffer.Initialize();
    spectrumData.lastBufferSize = 0;

    glGenVertexArrays(1, &borderData.vao);
    glBindVertexArray(borderData.vao);

    borderData.positionBuffer.Initialize();
    borderData.colorBuffer.Initialize();
    float displayScale = 6.0f;
    float displayXOffset = -10.0f;
    borderData.positionBuffer.vertices.push_back(glm::vec3(-displayScale + displayXOffset, -displayScale, -30.0f));
    borderData.positionBuffer.vertices.push_back(glm::vec3(-displayScale + displayXOffset, displayScale, -30.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

    borderData.positionBuffer.vertices.push_back(glm::vec3(-displayScale + displayXOffset, -displayScale, -30.0f));
    borderData.positionBuffer.vertices.push_back(glm::vec3(displayScale + displayXOffset, -displayScale, -30.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

    borderData.positionBuffer.vertices.push_back(glm::vec3(-displayScale + displayXOffset, displayScale, -30.0f));
    borderData.positionBuffer.vertices.push_back(glm::vec3(displayScale + displayXOffset, displayScale, -30.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

    borderData.positionBuffer.vertices.push_back(glm::vec3(displayScale + displayXOffset, -displayScale, -30.0f));
    borderData.positionBuffer.vertices.push_back(glm::vec3(displayScale + displayXOffset, displayScale, -30.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    borderData.colorBuffer.vertices.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

    enabled = true;

    return true;
}

void FrequencySpectrum::Process(std::vector<unsigned char>* block)
{
    Logger::Log("Processing block of ", block->size(), " elements in filter '", GetName(), "'.");

    // Displays the IQ spectrum flattened on the XY plane.
    float displayScale = 5.0f;
    float offsetX = -16.0f;
    float offsetY = 0.0f;
    float zPos = -30.0f;

    unsigned int fftSize = 32768 * 2;
    std::vector<unsigned char> fftBuffer;
    fftBuffer.reserve(fftSize); // 2 ^ 14. or 2.3 kHz Hz per frequency window.

    // Just copy the first few samples with some very inaccurate decimation. TODO make an actual flow structure for processed data.
    unsigned int downscaleAmount = 1; // 8; FYI this leads to jitter as we're not properly decimating. It does provide a minimum amount of zoom though.
    for (unsigned int i = 0, j = 0; j < fftSize / 2; j++, i+= downscaleAmount)
    {
        fftBuffer.push_back((*block)[i * 2]);
        fftBuffer.push_back((*block)[i * 2 + 1]);
    }
    
    std::vector<float> reals;
    std::vector<float> imags;
    auto startTime = std::chrono::high_resolution_clock::now();
    // FourierTransform::ComplexDFT(fftBuffer, reals, imags);
    FourierTransform::ComplexFFT(fftBuffer, reals, imags);
    auto stopTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> time = (stopTime - startTime);
    Logger::Log("Performed a fourier transform of ", fftSize / 2, " complex elements in ", time.count(), " sec.");

    float minReal = std::numeric_limits<float>::max();
    float maxReal = std::numeric_limits<float>::min();
    float avgReal = 0;
    float minImag = std::numeric_limits<float>::max();
    float maxImag = std::numeric_limits<float>::min();
    float avgImag = 0;
    for (unsigned int i = 0; i < reals.size(); i++)
    {
        if (reals[i] < minReal)
        {
            minReal = reals[i];
        }

        if (reals[i] > maxReal)
        {
            maxReal = reals[i];
        }

        avgReal += reals[i];

        if (imags[i] < minImag)
        {
            minImag = imags[i];
        }

        if (imags[i] > maxImag)
        {
            maxImag = imags[i];
        }

        avgImag += imags[i];
    }

    avgReal /= reals.size();
    avgImag /= imags.size();
    Logger::Log("Min, Max -- Re: [", minReal, ", ", maxReal, "], Im: [", minImag, ", ", maxImag, "]");
    avgReal = (avgReal - minReal) / (maxReal - minReal);
    avgImag = (avgImag - minImag) / (maxImag - minImag);

    float upscaleFactor = 16.0f;

    graphicsUpdateLock.lock();
    spectrumData.positionBuffer.vertices.clear();
    spectrumData.colorBuffer.vertices.clear();

    for (unsigned int i = 0; i < reals.size(); i++)
    {
        float xPosition = offsetX + ((float)i / (float)reals.size()) * 2.0f * (displayScale + 1.0f);
        float percentReal = (reals[i] - minReal) / (maxReal - minReal);
        float yPositionReal = offsetY + percentReal * displayScale * upscaleFactor + 1.0f - (avgReal * displayScale * upscaleFactor); // wiggle factor

        float percentImag = (imags[i] - minImag) / (maxImag - minImag);
        float yPositionImag = offsetY + percentImag * displayScale * upscaleFactor - (displayScale) - (avgImag * displayScale * upscaleFactor);

        spectrumData.positionBuffer.vertices.push_back(glm::vec3(xPosition, yPositionReal, zPos));
        spectrumData.colorBuffer.vertices.push_back(glm::vec3(1.0f, 1.0f, 0.0f));

        spectrumData.positionBuffer.vertices.push_back(glm::vec3(xPosition, yPositionImag, zPos));
        spectrumData.colorBuffer.vertices.push_back(glm::vec3(1.0f, 0.0f, 1.0f));
    }

    graphicsUpdateLock.unlock();

    updateGraphics = true;
}

void FrequencySpectrum::Render(glm::mat4& projectionMatrix)
{
    if (updateGraphics)
    {
        if (firstUpdate)
        {
            glBindVertexArray(borderData.vao);

            borderData.positionBuffer.TransferToOpenGl();
            borderData.colorBuffer.TransferToOpenGl();

            firstUpdate = false;
        }

        // Graphics updates must be on the graphics thread.
        glBindVertexArray(spectrumData.vao);

        graphicsUpdateLock.lock();
        spectrumData.positionBuffer.TransferToOpenGl();
        spectrumData.colorBuffer.TransferToOpenGl();
        spectrumData.lastBufferSize = spectrumData.positionBuffer.vertices.size();
        graphicsUpdateLock.unlock();

        updateGraphics = false;
    }
    else if (spectrumData.lastBufferSize == 0)
    {
        return;
    }

    // Render the IQ spectrum
    glUseProgram(spectrumProgram.programId);
    glBindVertexArray(spectrumData.vao);

    glUniformMatrix4fv(spectrumProgram.projMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniform1f(spectrumProgram.transparencyLocation, 1.0f);
    glUniform1f(spectrumProgram.pointSizeLocation, 1.0f);

    glDrawArrays(GL_POINTS, 0, spectrumData.lastBufferSize);

    glBindVertexArray(borderData.vao);
    glDrawArrays(GL_LINES, 0, borderData.positionBuffer.vertices.size());

}

FrequencySpectrum::~FrequencySpectrum()
{
    StopFilter();

    glDeleteVertexArrays(1, &spectrumData.vao);
    spectrumData.positionBuffer.Deinitialize();
    spectrumData.colorBuffer.Deinitialize();

    // FYI base destructor is automatically called, which doesn't match with how other languages handle base classes.
}
