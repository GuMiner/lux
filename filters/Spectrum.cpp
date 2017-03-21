#include <algorithm>
#include "Spectrum.h"

Spectrum::Spectrum(glm::vec2 startPosition, glm::vec2 startSize, SdrBuffer* dataBuffer)
    : lastPosition(startPosition), lastSize(startSize),
      updateGraphics(false), spectrumLines(true),
      windowedSincFilter(), FilterBase(dataBuffer)
{
    FormDecimator();
    enabled = true;
}

// Let's see the IQ graph only within a 3.71 kHz bandwidth. With our current sample rate of 2.4 MHz, we will decimate 648 samples for 1 sample of data.
void Spectrum::FormDecimator()
{
    windowedSincFilter.CreateFilter(3710, 648);
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

std::string Spectrum::GetName() const
{
    return "Spectrum";
}

void Spectrum::Process(std::vector<unsigned char>* block)
{
    // Logger::Log("Processing block of ", block->size(), " elements in filter '", GetName(), "'.");

    // Displays the IQ spectrum flattened on the XY plane.
    float scale = (1.0f / 127.5f) * std::min(lastSize.x, lastSize.y);

    graphicsUpdateLock.lock();
    spectrumLines.Clear();
    Logger::Log("Size block:", block->size(), " decimator:", windowedSincFilter.kernel.size(), ".");
    int samples = block->size() / 2;
    int steps = (samples - windowedSincFilter.kernel.size()) / windowedSincFilter.kernel.size();
    int counter = 0;
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
        
        float amplitude = std::sqrt(i * i + q * q);
        amplitude = std::min(amplitude, lastSize.y / 2.0f);
        amplitude = std::max(amplitude, -lastSize.y / 2.0f);
        
        spectrumLines.positionBuffer.vertices.push_back(glm::vec3(lastPosition.x + ((float)counter / (float)steps) * lastSize.x, lastPosition.y + amplitude + lastSize.y / 2.0f, 0.0f));
        spectrumLines.colorBuffer.vertices.push_back(glm::vec3(0.50f, 0.50f, 1.0f));
        ++counter;
    }

    graphicsUpdateLock.unlock();
    updateGraphics = true;
}

bool Spectrum::HasTitleUpdate()
{
    return false;
}

std::string Spectrum::GetTitle()
{
    return "Samples";
}

void Spectrum::Update(float elapsedTime, float frameTime)
{
    if (updateGraphics)
    {
        graphicsUpdateLock.lock();
        spectrumLines.Update();
        graphicsUpdateLock.unlock();

        updateGraphics = false;
    }
}

void Spectrum::Render(glm::mat4 & projectionMatrix, glm::vec2 position, glm::vec2 size)
{
    lastPosition = position;
    lastSize = size;

    spectrumLines.Render(projectionMatrix);
}

Spectrum::~Spectrum()
{
    StopFilter();
}
