#pragma once
#include <vector>

// Performs customized filter operations on a subset of data in various situations.
class CustomFilter
{
protected:
    // Computes a point on the following windows.
    float ComputeHammingWindowPt(int kernelIdx, int kernelLength);
    float ComputeBlackmanWindowPt(int kernelIdx, int kernelLength);

public:
    CustomFilter();

    // Fills in the pre-allocated kernel with the filter to use.
    virtual void CreateFilter(int kernelLength) = 0;

    // TODO This shouldn't be public, will refactor later.
    // The filter kernel to be applied to the data set.
    std::vector<float> kernel;
};

