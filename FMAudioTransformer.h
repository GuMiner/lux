#pragma once
#include "IAudioTransformer.h"

class FMAudioTransformer : public IAudioTransformer
{
public:
    FMAudioTransformer();
    ~FMAudioTransformer();

    // Inherited via IAudioTransformer
    virtual void Process(std::vector<unsigned char>* samples, std::vector<sf::Int16>* destinationBuffer) override;
};

