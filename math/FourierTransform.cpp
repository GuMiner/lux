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

unsigned int FourierTransform::reverseBits(unsigned int x, unsigned int maxValue)
{
    unsigned int maxShiftRange = log2(maxValue) - 1;
    unsigned int result = 0;
    for (unsigned int i = 0; i < maxShiftRange; i++)
    {
        unsigned int mask = 1 << i;
        if ((x & mask) != 0)
        {
            result |= 1 << (maxShiftRange - (i + 1));
        }
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
    if (length == 0 || (length & (length - 1)) != 0)
    {
        Logger::Log("Could not perform the complex FFT on a non-power of 2 sample length: ", samples.size());
        return false;
    }

    // Interlace our data.
    reals.clear();
    reals.reserve(length);

    imags.clear();
    imags.reserve(length);

    for (unsigned int i = 0; i < length; i++)
    {
        unsigned int idxToCopyFrom = FourierTransform::reverseBits(i, length);

        reals.push_back((float)samples[idxToCopyFrom * 2]);
        imags.push_back((float)samples[idxToCopyFrom * 2 + 1]);
    }

    int levels = FourierTransform::log2(length);
    for (int level = 0; level < levels; level++)
    {
        int subDftSize = (int)std::pow(2, level); // Go from small DFT to large DFT
        float ur = 1;
        float ui = 0;
        float sr = std::cos(Constants::PI / (float)subDftSize);
        float si = -std::sin(Constants::PI / (float)subDftSize);
        for (int j = 1; j < subDftSize; j++) // Iterate over the size of a DFT, skipping the first point
        {
            for (int i = j - 1; i <= (int)length - subDftSize; i += subDftSize*2) // Iterate over the entire data set, skipping the first point and iterating per DFT size
            {
                int ip = i + subDftSize;
                
                // X [ip] = (x[i] - x[ip]) * U, swap/add also
                float tr = reals[ip] * ur - imags[ip] * ui;
                float ti = reals[ip] * ui + imags[ip] * ur;
                reals[ip] = reals[i] - tr;
                imags[ip] = imags[i] - ti;
                reals[i] += tr;
                imags[i] += ti;
            }

            // U = U * S (complex mult)
            ur = ur * sr - ui * si;
            ui = ur * si + ui * sr;
        }
    }

    return true;
}
