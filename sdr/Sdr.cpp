#include <sstream>
#include "logging\Logger.h"
#include "Sdr.h"

std::vector<glm::ivec2> Sdr::ValidSampleRateRanges = std::vector<glm::ivec2>(
{
    glm::ivec2(225001, 300000),
    glm::ivec2(900001, 2400000),
    glm::ivec2(240000, 3200000) // Range at which we expect to loose samples.
});

std::vector<glm::ivec2> Sdr::ValidFrequencyRanges = std::vector<glm::ivec2>(
{
    glm::ivec2(24000000, 1766000000)
});

Sdr::Sdr()
    : rawLayer(), loadedDevices()
{
    
}

bool Sdr::Initialize()
{
    if (!rawLayer.Initialize())
    {
        return false;
    }

    Logger::Log("SDR initialized!");
    deviceCounts = rawLayer.get_device_count();
    Logger::Log("There ", deviceCounts == 1 ? "is" : "are", " currently ", deviceCounts, 
        " SDR device", deviceCounts == 1 ? "" : "s", " attached.");

    Logger::Log("Devices: ");
    for (unsigned int i = 0; i < 1; i++)
    {
        Logger::Log("  ", GetDeviceName(i));
    }

    return true;
}

bool Sdr::OpenDevice(int deviceId)
{
    if (loadedDevices.find(deviceId) != loadedDevices.end())
    {
        Logger::Log("Device ", deviceId, " is already loaded!");
        return true;
    }

    void* device;
    if (rawLayer.open(&device, deviceId) != 0)
    {
        Logger::Log("Couldn't open device ", deviceId, ".");
        return false;
    }

    char manufacturer[256];
    char product[256];
    char serial[256];
    if (rawLayer.get_device_usb_strings(deviceId, &manufacturer[0], &product[0], &serial[0]) != 0)
    {
        Logger::LogWarn("Couldn't get device details for device ", deviceId, ".");
        // This isn't unexpected, unfortunately. // return false;
    }
    else
    {
        Logger::Log("Opened device ", deviceId, ", made by ", manufacturer, " product ", product, ", serial ", serial, ".");
    }

    std::vector<int> gains;
    int numberOfGainValues = rawLayer.get_tuner_gains(device, nullptr);
    if (numberOfGainValues <= 0)
    {
        Logger::Log("Couldn't get the number of tuner gain settings available!");
        return false;
    }
    Logger::Log("Received ", numberOfGainValues, " possible gain values.");

    gains.reserve(numberOfGainValues);
    if (rawLayer.get_tuner_gains(device, &gains[0]) <= 0)
    {
        Logger::Log("Couldn't get the tuner gain settings available!");
        return false;
    }

    std::stringstream possibleGainValues;
    for (unsigned int i = 0; i < gains.size(); i++)
    {
        possibleGainValues << i;
        if (i != gains.size() - 1)
        {
            possibleGainValues << ", ";
        }
    }
    Logger::Log("Possible gain values are: ", possibleGainValues.str());

    loadedDevices[deviceId] = DeviceDetails(device, gains);
    return true;
}

int Sdr::GetDeviceCounts() const
{
    return deviceCounts;
}

const char* Sdr::GetDeviceName(int deviceId)
{
    return rawLayer.get_device_name(deviceId);
}

bool Sdr::SetCenterFrequency(int deviceId, unsigned int frequency)
{
    return (rawLayer.set_center_freq(loadedDevices[deviceId].device, frequency) == 0) ? true : false;
}

unsigned int Sdr::GetCenterFrequency(int deviceId)
{
    return rawLayer.get_center_freq(loadedDevices[deviceId].device);
}

int Sdr::GetFrequencyCorrection(int deviceId)
{
    return rawLayer.get_freq_correction(loadedDevices[deviceId].device);
}

bool Sdr::SetFrequencyCorrection(int deviceId, int ppm)
{
    return (rawLayer.set_freq_correction(loadedDevices[deviceId].device, ppm) == 0) ? true : false;
}

bool Sdr::SetTunerBandwidth(int deviceId, unsigned int bandwidth)
{
    return (rawLayer.set_tuner_bandwidth(loadedDevices[deviceId].device, bandwidth) == 0) ? true : false;
}

unsigned int Sdr::GetSampleRate(int deviceId)
{
    return (rawLayer.get_sample_rate(loadedDevices[deviceId].device));
}

bool Sdr::SetSampleRate(int deviceId, unsigned int rate)
{
    return (rawLayer.set_sample_rate(loadedDevices[deviceId].device, rate) == 0) ? true : false;
}

int Sdr::GetTunerGain(int deviceId)
{
    int gainValue = rawLayer.get_tuner_gain(loadedDevices[deviceId].device);
    if (gainValue == 0)
    {
        return -1;
    }

    for (unsigned int i = 0; i < loadedDevices[deviceId].gains.size(); i++)
    {
        if (loadedDevices[deviceId].gains[i] == gainValue)
        {
            return i;
        }
    }

    return -1;
}

bool Sdr::SetTunerGain(int deviceId, int gainId)
{
    return (rawLayer.set_tuner_gain(loadedDevices[deviceId].device, loadedDevices[deviceId].gains[gainId]) == 0) ? true : false;
}

std::vector<int> Sdr::GetTunerGainSettings(int deviceId)
{
    return loadedDevices[deviceId].gains;
}

bool Sdr::SetTunerGainMode(int deviceId, bool manualMode)
{
    return (rawLayer.set_tuner_gain_mode(loadedDevices[deviceId].device, manualMode ? 1 : 0) == 0) ? true : false;
}

bool Sdr::SetInternalAutoGain(int deviceId, bool enable)
{
    return (rawLayer.set_agc_mode(loadedDevices[deviceId].device, enable ? 1 : 0) == 0) ? true : false;
}

bool Sdr::ResetBuffer(int deviceId)
{
    return (rawLayer.reset_buffer(loadedDevices[deviceId].device) == 0) ? true : false;
}

bool Sdr::ReadBlock(int deviceId, unsigned char* buffer, unsigned int blocks, int* bytesRead)
{
    return (rawLayer.read_sync(loadedDevices[deviceId].device, buffer, blocks * BLOCK_SIZE, bytesRead) == 0 ? true : false);
}

Sdr::~Sdr()
{
    // Close all devices loaded.
    for (auto iter = loadedDevices.cbegin(); iter != loadedDevices.cend(); iter++)
    {
        if (rawLayer.close(iter->second.device) != 0)
        {
            Logger::LogError("Couldn't close device ID ", iter->first);
        }
    }
}