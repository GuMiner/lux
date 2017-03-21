#pragma once
#include <vector>
#include <SFML\Audio.hpp>

// Transforms raw samples into audio samples.
class IAudioTransformer
{
public:
    // Processes a block of samples, storing them into a destination buffer.
    virtual void Process(std::vector<unsigned char>* samples, std::vector<sf::Int16>* destinationBuffer) = 0;
};
