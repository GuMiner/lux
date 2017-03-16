#include "math\Constants.h"
#include "FourierTransform.h"

unsigned int FourierTransform::log2(unsigned int x)
{
    int result = 0;
    while (x != 0)
    {
        x >>= 1;
        ++result;
    }

    return result;
}

unsigned int FourierTransform::reverseBits(unsigned int x)
{
    unsigned int uintSize = sizeof(int) * 8;
    unsigned int result = 0;
    for (unsigned int i = 0; i < uintSize; i++)
    {
        unsigned int mask = 1 << i;
        result |= (x & mask) << (uintSize - (1 + i));
    }

    return result;
}

bool FourierTransform::ComplexDFT(std::vector<unsigned char> samples, std::vector<float>& reals, std::vector<float>& imags)
{
    unsigned int length = samples.size() / 2;

    // Prepare our output arrays
    reals.clear();
    reals.resize(length, 0);

    imags.clear();
    imags.resize(length, 0);

    for (unsigned int i = 0; i < length; i++)
    {
        for (unsigned int j = 0; j < length; j++)
        {
            float angularFrequency = 2.0f * Constants::PI * (float)i * (float)j / (float)length;
            float realCosine = std::cos(angularFrequency);
            float imagSine = -std::sin(angularFrequency);
            reals[i] += samples[j * 2] * realCosine - samples[j * 2 + 1] * imagSine;
            imags[i] += samples[j * 2] * imagSine   + samples[j * 2 + 1] * realCosine;
        }
    }

    return true;
}

bool FourierTransform::ComplexFFT(std::vector<unsigned char> samples, std::vector<float>& reals, std::vector<float>& imags)
{
    unsigned int length = samples.size() / 2;
    if (length == 0 || (length & (length - 1)) == 0)
    {
        Logger::Log("Could not perform the complex FFT on a non-power of 2 sample length: ", samples.size());
        return false;
    }

    // V1 -- uses a separate data copy to be safe.
    std::vector<unsigned char> copy;
    copy.reserve(samples.size());

    // Interlace our data.
    for (unsigned int i = 0; i < length; i++)
    {
        unsigned int idxToCopyFrom = FourierTransform::reverseBits(i);
        copy.push_back(samples[idxToCopyFrom * 2]);
        copy.push_back(samples[idxToCopyFrom * 2 + 1]);
    }

    // Prepare our output arrays
    reals.clear();
    reals.resize(length, 0);

    imags.clear();
    imags.resize(length, 0);

    int levels = FourierTransform::log2(length);
    for (int level = 0; level < levels; level++)
    {
        // TODO complete not at 10 PM.
    }

    return true;
}
