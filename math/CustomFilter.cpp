#include <algorithm>
#include "Constants.h"
#include "CustomFilter.h"

float CustomFilter::ComputeHammingWindowPt(int kernelIdx, int kernelLength)
{
    // The Scientist and Engineer's Guide to DSP, Ch 16.
    return 0.54f - 0.46f * std::cos((float)(2 * Constants::PI * kernelIdx) / (float)kernelLength);
}

float CustomFilter::ComputeBlackmanWindowPt(int kernelIdx, int kernelLength)
{
    // The Scientist and Engineer's Guide to DSP, Ch 16.
    float angle = (float)(2 * Constants::PI * kernelIdx) / (float)kernelLength;
    return 0.42f - 0.50f * std::cos(angle) + 0.08f * std::cos(2 * angle);
}

CustomFilter::CustomFilter()
{
}
