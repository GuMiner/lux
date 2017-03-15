#include <algorithm>
#include "GuCommon\logging\Logger.h"
#include "Constants.h"
#include "CustomFilter.h"
#include "WindowedSincFilter.h"

float WindowedSincFilter::ComputeWindowedSincPt(float cutoffFrequency, int kernelIdx, int kernelLength)
{
    int currentId = kernelIdx - kernelLength / 2;
    if (currentId == 0)
    {
        return 2 * Constants::PI * cutoffFrequency;
    }

    return std::sin(2 * Constants::PI * cutoffFrequency * (float)currentId) / (float)currentId;
}

void WindowedSincFilter::CreateFilter(int kernelLength)
{
    float sum = 0;
    for (int i = 0; i < kernelLength; i++)
    {
        float kernelValue = ComputeWindowedSincPt(cutoffFrequency, i, kernelLength);
        sum += kernelValue;
        kernel.push_back(kernelValue);
    }

    for (int i = 0; i < kernelLength; i++)
    {
        kernel[i] /= sum;
    }
}

void WindowedSincFilter::CreateFilter(float cutoffFrequency, int kernelLength)
{
    this->cutoffFrequency = cutoffFrequency;
    kernel.reserve(kernelLength);
    CreateFilter(kernelLength);
}

WindowedSincFilter::WindowedSincFilter() : CustomFilter()
{
}
