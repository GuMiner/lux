#pragma once
#include <vector>
#include "logging\Logger.h"

// Performs the Fourier Transform for a variety of inputs.
class FourierTransform
{
    static unsigned int log2(unsigned int x);

    static unsigned int FourierTransform::reverseBits(unsigned int x, unsigned int maxValue);

public:
    // Performs the Complex DFT on a series of inputs, returning a vector of reals and imaginaries.
    // This should be identical to the FFT, but run more slowly.
    static bool ComplexDFT(std::vector<unsigned char> samples, std::vector<float>& reals, std::vector<float>& imags);

    // Performs the Complex FFT on a series of inputs, returning a vector of reals and imaginaries.
    static bool ComplexFFT(std::vector<unsigned char> samples, std::vector<float>& reals, std::vector<float>& imags);
};

