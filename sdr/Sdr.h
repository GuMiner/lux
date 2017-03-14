#pragma once
#include <map>
#include <vector>
#include <glm\vec2.hpp>
#include "RtlSdrDllLoader.h"

struct DeviceDetails
{
    void* device;
    std::vector<int> gains;

    DeviceDetails(void* device, std::vector<int> gains)
        : device(device), gains(gains)
    {
    }

    DeviceDetails()
        : device(nullptr), gains()
    {
    }
};

// Defines an SDR interface abstracting away the device. TODO more abstraction once I have usable algorithms.
class Sdr
{
    void* device;
    RtlSdrDllLoader rawLayer;
    
    unsigned int deviceCounts;
    std::map<int, DeviceDetails> loadedDevices;
public:
    static const unsigned int BLOCK_SIZE = 16384;
    static std::vector<glm::ivec2> ValidSampleRateRanges;
    static std::vector<glm::ivec2> ValidFrequencyRanges;

    Sdr();
    
    bool Initialize();

    // Both of these are safe to call before loading any devices.
    int GetDeviceCounts() const;
    const char* GetDeviceName(int deviceId);

    bool OpenDevice(int deviceId);
    
    // Frequency functionality
    bool SetCenterFrequency(int deviceId, unsigned int frequency);
    unsigned int GetCenterFrequency(int deviceId);
    int GetFrequencyCorrection(int deviceId);
    bool SetFrequencyCorrection(int deviceId, int ppm);

    // Bandwidth functionality
    bool SetTunerBandwidth(int deviceId, unsigned int bandwidth);
    unsigned int GetSampleRate(int deviceId);
    bool SetSampleRate(int deviceId, unsigned int rate);

    // Gain functionality
    int GetTunerGain(int deviceId); // Returns the ID of the gain within the vector, -1 on failure.
    bool SetTunerGain(int deviceId, int gainId); // ID of the gain within the vector.
    std::vector<int> GetTunerGainSettings(int deviceId);
    bool SetTunerGainMode(int deviceId, bool manualMode);
    bool SetInternalAutoGain(int deviceId, bool enable);
    
    // Acquisition functionality
    // Useful information on the IQ format: http://whiteboard.ping.se/SDR/IQ
    // Format: [I, Q] [I, Q] ... [In, Qn] Subtract 127 to get the actual value (8-bit accuracy)
    bool ResetBuffer(int deviceId);
    bool ReadBlock(int deviceId, unsigned char* buffer, unsigned int blocks, int* bytesRead);

    ~Sdr();
};
