#pragma once
#include "CustomFilter.h"

// Performs customized filter operations on a subset of data in various situations.
class WindowedSincFilter : public CustomFilter
{
    float cutoffFrequency;

    float ComputeWindowedSincPt(float cutoffFrequency, int kernelIdx, int kernelLength);
    
    // Overrides CreateFilter to create a WindowedSinc filter.
    virtual void CreateFilter(int kernelLength) override;

public:
    WindowedSincFilter();

    void CreateFilter(float cutoffFrequency, int kernelLength);
};

