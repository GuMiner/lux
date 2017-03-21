#include "FMAudioTransformer.h"



FMAudioTransformer::FMAudioTransformer()
{
}


FMAudioTransformer::~FMAudioTransformer()
{
}

void FMAudioTransformer::Process(std::vector<unsigned char>* samples, std::vector<sf::Int16>* destinationBuffer)
{
    // TODO figure out the best way to deserialize FM stereo audio.
}
