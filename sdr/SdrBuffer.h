#pragma once
#include <atomic>
#include <future>
#include <map>
#include <vector>
#include <glm\vec2.hpp>
#include <SFML\System.hpp>
#include "logging\Logger.h"
#include "Sdr.h"

// Defines a buffer to continually receive data from the SDR device.
// TODO break this apart correctly from CPP to H when it isn't too late, make it easy for a consumer to retrieve the data in nice blocks, and actually display the data.
class SdrBuffer
{
    Sdr* sdrDevice;
    unsigned int deviceId;

    std::atomic<bool> isAcquiring;
    std::atomic<bool> isTerminating;
    std::future<void> dataAcquisitionThread;
    
    // Transient data storage.
    unsigned int bufferBlocks;
    unsigned int bufferBlockReadSize;
    std::vector<unsigned char> rollingBuffer;

    // Data from start to here is valid, exclusive.
    unsigned int currentBufferPosition;

    // Data from here to end is *invalid*, inclusive.
    unsigned int endBufferPosition;
    

    // Used to compute how fast we roll through the rolling buffer.
    std::atomic<float> elapsedTime;
    std::atomic<unsigned int> acquiredSamples;
    std::atomic<float> dataSampleRate;
public:
    
    // BlockReadSize is recommended to be 16, blocks should be a multiple of the read size for best performance (ie, 80)
    SdrBuffer(Sdr* sdrDevice, unsigned int deviceId, unsigned int blocks, unsigned int blockReadSize)
        : sdrDevice(sdrDevice), deviceId(deviceId), isAcquiring(false), isTerminating(false), 
          bufferBlocks(blocks), bufferBlockReadSize(blockReadSize),
          rollingBuffer(), currentBufferPosition(0), endBufferPosition(Sdr::BLOCK_SIZE * blocks - 1),
          elapsedTime(0.0f), acquiredSamples(0), dataSampleRate(0.0f)
    {
        rollingBuffer.reserve(Sdr::BLOCK_SIZE * blocks);
    }
    
    float GetCurrentSampleRate() const
    {
        return dataSampleRate;
    }

    unsigned int GetBufferSize() const
    {
        return Sdr::BLOCK_SIZE * bufferBlocks;
    }

    unsigned int GetReadSize() const
    {
        return Sdr::BLOCK_SIZE * bufferBlockReadSize;
    }

    void AdvanceBufferPositions(unsigned int bytesRead)
    {
        currentBufferPosition += (bytesRead + 1);
        if (GetBufferSize() - GetReadSize() < currentBufferPosition)
        {
            endBufferPosition = currentBufferPosition;
            currentBufferPosition = 0;
        }
    }

    void StartAcquisition()
    {
        if (isAcquiring)
        {
            return;
        }

        // Slepp for at most 1/2 a second. If we aren't done by then, we permit overwriting our thread
        //  potentially causing data corruption until the other thread terminates.
        for (int i = 0; i < 50 && isTerminating; i++)
        {
            sf::sleep(sf::milliseconds(10));
        }

        isAcquiring = true;

        Logger::Log("Starting data acquisition: ", isAcquiring.load(), " ", isTerminating.load());
        dataAcquisitionThread = std::async(std::launch::async, &SdrBuffer::AcquireData, this);
    }

    void AcquireData()
    {
        Logger::Log("Resetting RTL-SDR buffer to read data: ", sdrDevice->ResetBuffer(0));

        while (isAcquiring)
        {
            sf::Clock clock;

            int bytesRead = 0;
            if (!sdrDevice->ReadBlock(deviceId, &rollingBuffer[currentBufferPosition], bufferBlockReadSize, &bytesRead))
            {
                Logger::Log("Error reading from the SDR device: ", deviceId);
            }

            AdvanceBufferPositions(bytesRead);

            // Compute how fast we're acquiring samples.
            acquiredSamples = acquiredSamples + bytesRead;
            elapsedTime = elapsedTime + clock.getElapsedTime().asSeconds();
            if (elapsedTime > 1.0f)
            {
                dataSampleRate = acquiredSamples / elapsedTime;
                acquiredSamples = 0;
                elapsedTime = 0;
            }

        }

        isTerminating = false;
    }

    void StopAcquisition()
    {
        if (!isAcquiring)
        {
            return;
        }

        isTerminating = true;
        isAcquiring = false;
        Logger::Log("Stopping data acquisition: ", isAcquiring.load(), " ", isTerminating.load());
    }
};
