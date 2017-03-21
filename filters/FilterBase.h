#pragma once
#include <chrono>
#include <future>
#include <thread>
#include "sdr\SdrBuffer.h"

// Defines the base class for filter operations performed on I/Q SDR data
class FilterBase
{
    SdrBuffer* dataBuffer;
    unsigned int localBlockId;

    std::vector<unsigned char> currentBlock;

    bool acquiringBlocks;
    std::future<void> acquisitionThread;

    // Continually acquire blocks, sending them to the associated process function in a mirrored data set.
    void AcquireBlocks()
    {
        bool wasEnabled = true;
        while (acquiringBlocks)
        {
            if (enabled)
            {
                if (!wasEnabled)
                {
                    localBlockId = dataBuffer->GetCurrentBlockId();
                    wasEnabled = true;
                }

                while (acquiringBlocks && localBlockId != dataBuffer->GetCurrentBlockId())
                {
                    // Acquire and process the new block.
                    int idx = (localBlockId % dataBuffer->GetReadBlocks());
                    int startOffset = idx * dataBuffer->GetReadSize();
                    int endOffset = (idx + 1) * dataBuffer->GetReadSize();
                    currentBlock.clear();
                    currentBlock.insert(currentBlock.begin(), dataBuffer->GetBuffer().begin() + startOffset, dataBuffer->GetBuffer().begin() + endOffset);
                    
                    // Logger::Log("Filter '", GetName(), "' processing new block ID ", (int)localBlockId, ".");
                    this->Process(&currentBlock);
                    
                    ++localBlockId;
                }

                // Sleep for a half of the time it takes to acquire a block, so we don't leave the processing step with insufficient leeway in time.
                float currentSampleRate = dataBuffer->GetCurrentSampleRate();
                if (currentSampleRate > 1e4)
                {
                    // If the sample rate is lower, we're still initializing. Don't wait.
                    float sleepInMicroseconds = (1e6f * (float)dataBuffer->GetReadSize()) / (currentSampleRate * 2.0f);
                    // Logger::Log("Filter '", GetName(), "' sleeping for ", sleepInMicroseconds, " us.");
                    std::this_thread::sleep_for(std::chrono::microseconds((long)sleepInMicroseconds));
                }
            }
            else
            {
                wasEnabled = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                Logger::Log("Sleeing for 500 ms as the '", GetName(), "' filter was disabled.");
            }
        }
    }

protected:
    bool enabled;

    void StopFilter()
    {
        if (acquiringBlocks)
        {
            acquiringBlocks = false;
            acquisitionThread.wait();
        }
    }

public:
    FilterBase(SdrBuffer* dataBuffer)
        : dataBuffer(dataBuffer), acquiringBlocks(true)
    {
        localBlockId = dataBuffer->GetCurrentBlockId();
        currentBlock.reserve(dataBuffer->GetReadSize());

        enabled = false;
        acquisitionThread = std::async(std::launch::async, &FilterBase::AcquireBlocks, this);
    }

    virtual std::string GetName() const = 0;

    // Performs filter-specific short-term processing. Called for every new block.
    // Processes that need several blocks, or are expected to acquire blocks and then
    //  perform extensive processing them (discarding or caching new blocks during the extensive processing),
    //  should use a dedicated thread to perform said processing.
    virtual void Process(std::vector<unsigned char>* block) = 0;

    virtual ~FilterBase()
    {
        StopFilter();
    }
};

