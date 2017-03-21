#pragma once
#include "IAudioTransformer.h"
#include "math\WindowedSincFilter.h"

class AMAudioTransformer : public IAudioTransformer
{
    WindowedSincFilter downsamplingFilter;

public:
    AMAudioTransformer();
    ~AMAudioTransformer();

    // Inherited via IAudioTransformer
    virtual void Process(std::vector<unsigned char>* samples, std::vector<sf::Int16>* destinationBuffer) override;
};

