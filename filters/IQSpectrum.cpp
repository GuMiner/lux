#include <algorithm>
#include "IQSpectrum.h"

IQSpectrum::IQSpectrum(glm::vec2 startPosition, glm::vec2 startSize, SdrBuffer* dataBuffer)
    : lastPosition(startPosition), lastSize(startSize),
      updateGraphics(false), iqPoints(),
      windowedSincFilter(), FilterBase(dataBuffer)
{
    FormDecimator();
    enabled = true;
}

// Let's see the IQ graph only within a 3.71 kHz bandwidth. With our current sample rate of 2.4 MHz, we will decimate 648 samples for 1 sample of data.
void IQSpectrum::FormDecimator()
{
    windowedSincFilter.CreateFilter(3710, 1024);
    float sum = 0;
    for (unsigned int i = 0; i < windowedSincFilter.kernel.size(); i++)
    {
        // TODO make a filter display pane that takes CustomFilter types for display.
        float offsetX = -10.0f;
        float offsetY = -6.0f;
        float zPos = -30.0f;
        float xEffective = offsetX + ((float)i / (float)windowedSincFilter.kernel.size()) * 8.0f;
    }

    Logger::Log("Formed windowed sinc filter for future decimation.");
}

std::string IQSpectrum::GetName() const
{
    return "IQ Spectrum";
}

void IQSpectrum::Process(std::vector<unsigned char>* block)
{
    // Logger::Log("Processing block of ", block->size(), " elements in filter '", GetName(), "'.");

    // Displays the IQ spectrum flattened on the XY plane.
    float scale = (1.0f / 127.5f) * std::min(lastSize.x, lastSize.y);

    graphicsUpdateLock.lock();
    iqPoints.Clear();
    Logger::Log("Size block:", block->size(), " decimator:", windowedSincFilter.kernel.size(), ".");
    int samples = block->size() / 2;
    for (unsigned int n = 0; n < samples - windowedSincFilter.kernel.size(); n += windowedSincFilter.kernel.size()) // n < block->size() / 2; n++)
    {
        // TODO determine how to properly decimate IQ signals.
        // TODO this leads to discontinuities at edges. We should grab two buffers and process that to avoid boundary problems.
        float i = 0;
        float q = 0;
        for (unsigned int m = 0; m < windowedSincFilter.kernel.size(); m++)
        {
            // TODO there are weird artefacts using these decimation factors (scaling & descaling).s
            i += ((float)(*block)[(n + m) * 2] - 127.5f) * scale * windowedSincFilter.kernel[m];
            q += ((float)(*block)[(n + m) * 2 + 1] - 127.5f) * scale * windowedSincFilter.kernel[m];
        }
        
        i = std::max(i, -lastSize.x / 2.0f);
        i = std::min(i, lastSize.x / 2.0f);
        q = std::max(q, -lastSize.y / 2.0f);
        q = std::min(q, lastSize.y / 2.0f);

        // i = ((float)(*block)[n * 2] - 127.5f) * scale;
        // q = ((float)(*block)[n * 2 + 1] - 127.5f) * scale;
        
        iqPoints.positionBuffer.vertices.push_back(glm::vec3(lastPosition.x + i, lastPosition.y + q, 0.0f) + glm::vec3(lastSize.x / 2.0f, lastSize.y / 2.0f, 0.0f));
        iqPoints.colorBuffer.vertices.push_back(glm::vec3(0.50f, 0.50f, 1.0f));
    }

    graphicsUpdateLock.unlock();
    updateGraphics = true;
}

bool IQSpectrum::HasTitleUpdate()
{
    return false;
}

std::string IQSpectrum::GetTitle()
{
    return "IQ Samples";
}

void IQSpectrum::Update(float elapsedTime, float frameTime)
{
    if (updateGraphics)
    {
        graphicsUpdateLock.lock();
        iqPoints.Update();
        graphicsUpdateLock.unlock();

        updateGraphics = false;
    }
}

void IQSpectrum::Render(glm::mat4 & projectionMatrix, glm::vec2 position, glm::vec2 size)
{
    lastPosition = position;
    lastSize = size;

    iqPoints.Render(projectionMatrix);
}

IQSpectrum::~IQSpectrum()
{
    StopFilter();
}
