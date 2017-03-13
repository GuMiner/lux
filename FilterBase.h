#pragma once
#include <chrono>
#include <thread>
#include <glm\gtc\matrix_transform.hpp>
#include "sdr\SdrBuffer.h"

// Defines the base class for filter operations performed on I/Q SDR data
class FilterBase
{
    SdrBuffer* dataBuffer;
    unsigned int localBlockId;

    std::vector<unsigned char> currentBlock;

    bool acquiringBlocks;

    // Continually acquire blocks, sending them to the associated process function in a mirrored data set.
    void AcquireBlocks()
    {
        while (acquiringBlocks)
        {
            while (acquiringBlocks && localBlockId != dataBuffer->GetCurrentBlockId())
            {
                // Acquire and process the new block.
                int idx = (localBlockId % dataBuffer->GetReadBlocks());
                int startOffset = idx * dataBuffer->GetReadSize();
                int endOffset = (idx + 1) * dataBuffer->GetReadSize();
                currentBlock.insert(currentBlock.begin(), dataBuffer->GetBuffer().begin() + startOffset, dataBuffer->GetBuffer().begin() + endOffset);
                this->Process(&currentBlock);

                ++localBlockId;
            }

            // Sleep for a half of the time it takes to acquire a block, so we don't leave the processing step with insufficient leeway in time.
            float currentSampleRate = dataBuffer->GetCurrentSampleRate();
            if (currentSampleRate > 1e4)
            {
                // If the sample rate is lower, we're still initializing. Don't wait.
                float sleepInMicroseconds = (1e6 * (float)dataBuffer->GetReadSize()) / (currentSampleRate * 2.0f);
                std::this_thread::sleep_for(std::chrono::microseconds((long)sleepInMicroseconds));
            }
        }
    }

protected:
    
public:
    FilterBase(SdrBuffer* dataBuffer)
        : dataBuffer(dataBuffer), acquiringBlocks(true)
    {
        localBlockId = dataBuffer->GetCurrentBlockId();
        currentBlock.reserve(dataBuffer->GetReadSize());
    }

    // Performs filter-specific short-term processing. Called for every new block.
    // Processes that need several blocks, or are expected to acquire blocks and then
    //  perform extensive processing them (discarding or caching new blocks during the extensive processing),
    //  should use a dedicated thread to perform said processing.
    virtual void Process(std::vector<unsigned char>* block) = 0;

    // Renders graphical aspects associated with the filter.
    virtual void Render(glm::mat4& projectionMatrix) = 0;

    ~FilterBase()
    {
        acquiringBlocks = false;
    }
};

