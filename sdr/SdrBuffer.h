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
    const unsigned int bufferBlockReadSize = 16;

    Sdr* sdrDevice;
    unsigned int deviceId;

    std::atomic<bool> isAcquiring;
    std::atomic<bool> isTerminating;
    std::future<void> dataAcquisitionThread;
    
    // Transient data storage.
    unsigned int readBlocks;
    unsigned int bufferBlocks;
    std::vector<unsigned char> rollingBuffer;

    // Data from start to here is valid, exclusive.
    std::atomic<unsigned int> blockId;
    unsigned int currentBufferPosition;

    // Used to compute how fast we roll through the rolling buffer.
    std::atomic<float> elapsedTime;
    std::atomic<unsigned int> acquiredSamples;
    std::atomic<float> dataSampleRate;
public:
    
    // BlockReadSize is recommended to be 16, blocks should be a multiple of the read size for best performance (ie, 80)
    SdrBuffer(Sdr* sdrDevice, unsigned int deviceId, unsigned int bufferSize)
        : sdrDevice(sdrDevice), deviceId(deviceId), isAcquiring(false), isTerminating(false), 
          readBlocks(bufferSize), bufferBlocks(bufferSize * bufferBlockReadSize),
          rollingBuffer(), currentBufferPosition(0), blockId(0),
          elapsedTime(0.0f), acquiredSamples(0), dataSampleRate(0.0f)
    {
        Logger::Log("Creating a buffer of ", bufferBlocks, " blocks with a reads size of ", bufferBlockReadSize);
        rollingBuffer.reserve(Sdr::BLOCK_SIZE * bufferBlocks);
    }
    
    float GetCurrentSampleRate() const
    {
        return dataSampleRate;
    }

    unsigned int GetCurrentBlockId()
    {
        return blockId.load();
    }

    unsigned int GetReadSize() const
    {
        return Sdr::BLOCK_SIZE * bufferBlockReadSize;
    }

    unsigned int GetReadBlocks() const
    {
        return readBlocks;
    }

    const std::vector<unsigned char>& GetBuffer() const
    {
        return rollingBuffer;
    }

    void AdvanceBufferPositions()
    {
        ++blockId;
        currentBufferPosition += GetReadSize();
        if (currentBufferPosition > (Sdr::BLOCK_SIZE * bufferBlocks - GetReadSize()))
        {
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
            else if (bytesRead != this->GetReadSize())
            {
                Logger::LogWarn("Unable to read a full block from the SDR device. Read ", bytesRead, " bytes instead.");
                Logger::LogWarn("Block ID ", blockId.load(), " will be corrupted.");
            }

            AdvanceBufferPositions();

            // Compute how fast we're acquiring samples, averaged over a second.
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
