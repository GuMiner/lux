#include <Windows.h>
#include "logging\Logger.h"
#include "RtlSdrDllLoader.h"

// Avoid too much duplicate typing...
#define LoadFunction(functionName) \
    functionName = (rtlsdr_##functionName)GetProcAddress((HMODULE)dllHandle, "rtlsdr_" #functionName); \
    if (!functionName) \
    { \
        Logger::LogError("Couldn't load the rtlsdr_" #functionName " function: ", GetLastError()); \
        return false; \
    } \
    \
    Logger::Log("Loaded the " #functionName " function."); \
    \

RtlSdrDllLoader::RtlSdrDllLoader()
    : dllHandle(nullptr)
{
}

bool RtlSdrDllLoader::Initialize()
{
    // Load our DLL
    dllHandle = LoadLibraryW(L"rtlsdr.dll");
    if (!dllHandle)
    {
        Logger::LogError("Couldn't load the RTL-SDR DLL: ", GetLastError());
        return false;
    }

    // Load the necessary function pointers
    LoadFunction(get_device_count);
    LoadFunction(get_device_name);
    LoadFunction(get_device_usb_strings);
    LoadFunction(open);
    LoadFunction(close);
    LoadFunction(set_center_freq);
    LoadFunction(get_center_freq);
    LoadFunction(set_freq_correction);
    LoadFunction(get_freq_correction);
    LoadFunction(get_tuner_gains);
    LoadFunction(set_tuner_gain);
    LoadFunction(set_tuner_bandwidth);
    LoadFunction(get_tuner_gain);
    LoadFunction(set_tuner_gain_mode);
    LoadFunction(set_sample_rate);
    LoadFunction(get_sample_rate);
    LoadFunction(set_agc_mode);
    LoadFunction(reset_buffer);
    LoadFunction(read_sync);
    return true;
}

RtlSdrDllLoader::~RtlSdrDllLoader()
{
    if (dllHandle != nullptr)
    {
        FreeLibrary((HMODULE)dllHandle);
    }
}