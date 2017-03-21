#include <algorithm>
#include <chrono>
#include <limits>
#include "FrequencySpectrum.h"
#include "math\FourierTransform.h"

FrequencySpectrum::FrequencySpectrum(glm::vec2 startPosition, glm::vec2 startSize, SdrBuffer* dataBuffer)
    : lastPosition(startPosition), lastSize(startSize), updateGraphics(false), fftReals(), fftImags(), FilterBase(dataBuffer)
{
    enabled = true;
}

std::string FrequencySpectrum::GetName() const
{
    return "Frequency Spectrum";
}

void FrequencySpectrum::Process(std::vector<unsigned char>* block)
{
    // Logger::Log("Processing block of ", block->size(), " elements in filter '", GetName(), "'.");

    // Displays the IQ spectrum flattened on the XY plane.
    unsigned int fftSize = 1024 * 2;
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
    FourierTransform::ComplexDFT(fftBuffer, reals, imags);
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

    float upscaleFactor = 4.0f;

    graphicsUpdateLock.lock();
    fftReals.Clear();
    fftImags.Clear();

    for (unsigned int i = 0; i < reals.size(); i++)
    {
        float xPosition = lastPosition.x + ((float)i / (float)reals.size()) * lastSize.x;
        float percentReal = (reals[i] - minReal) / (maxReal - minReal);
        float yPositionReal = lastPosition.y + percentReal * lastSize.y * upscaleFactor;

        float percentImag = (imags[i] - minImag) / (maxImag - minImag);
        float yPositionImag = lastPosition.y + percentImag * lastSize.y * upscaleFactor;

        fftReals.positionBuffer.vertices.push_back(glm::vec3(xPosition, yPositionReal, 0.0f));
        fftReals.colorBuffer.vertices.push_back(glm::vec3(1.0f, 1.0f, 0.0f));

        fftImags.positionBuffer.vertices.push_back(glm::vec3(xPosition, yPositionImag, 0.0f));
        fftImags.colorBuffer.vertices.push_back(glm::vec3(1.0f, 0.0f, 1.0f));
    }

    graphicsUpdateLock.unlock();
    updateGraphics = true;
}

bool FrequencySpectrum::HasTitleUpdate()
{
    return false;
}

std::string FrequencySpectrum::GetTitle()
{
    return "Frequency Spectrum";
}

void FrequencySpectrum::Update(float elapsedTime, float frameTime)
{
    if (updateGraphics)
    {
        graphicsUpdateLock.lock();
        fftReals.Update();
        fftImags.Update();
        graphicsUpdateLock.unlock();
        
        updateGraphics = false;
    }
}

void FrequencySpectrum::Render(glm::mat4 & projectionMatrix, glm::vec2 position, glm::vec2 size)
{
    lastPosition = position;
    lastSize = size;

    fftReals.Render(projectionMatrix);
    fftImags.Render(projectionMatrix);
}

FrequencySpectrum::~FrequencySpectrum()
{
    StopFilter();
}
