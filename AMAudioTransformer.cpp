#include "Gucommon\logging\Logger.h"
#include "AMAudioTransformer.h"

AMAudioTransformer::AMAudioTransformer()
    : downsamplingFilter()
{
    // TODO use variables not constants.
    downsamplingFilter.CreateFilter(44100, 1024);
}


AMAudioTransformer::~AMAudioTransformer()
{
}

void AMAudioTransformer::Process(std::vector<unsigned char>* samples, std::vector<sf::Int16>* destinationBuffer)
{
    // We need to take one point per 54 points to reach approximately 44,100 samples/sec.
    for (unsigned int i = 0; i < samples->size() / 2; i += 40)
    {
        // TODO use filter here.

        // Max range ~= 180.
        // Amplitude of the I/Q stream as audio samples.
        float intMax = 32767;
        float max = 180.0f;
        float amplifier = 100.0f;
        float amplitude = amplifier * std::min(max, std::max(-max, std::sqrt(std::pow((float)((*samples)[i * 2]) - 127.5f, 2.0f) + std::pow((float)((*samples)[i * 2 + 1]) - 127.5f, 2.0f))));
        // TODO use variables not calculations


        // Simulate stereo by passing two samples per sample retrieved.
        sf::Int64 signal = (sf::Int16)((amplitude / max) * (intMax / max));
        destinationBuffer->push_back(signal);
        destinationBuffer->push_back(signal);
        if (i == 0)
        {
            Logger::Log("Signal: ", signal);
        }
    }

    Logger::Log("Loaded ", destinationBuffer->size(), " samples.");
}
