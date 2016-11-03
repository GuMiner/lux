#pragma once
#include <Windows.h>

typedef unsigned int(*rtlsdr_get_device_count)();
typedef const char*(*rtlsdr_get_device_name)(unsigned int index);
typedef int(*rtlsdr_get_device_usb_strings)(unsigned int index, char* manufacturer, char* product, char* serial);
typedef int(*rtlsdr_open)(void** device, unsigned int index);
typedef int(*rtlsdr_close)(void* device);
typedef int(*rtlsdr_set_center_freq)(void* device, unsigned int frequency);
typedef unsigned int(*rtlsdr_get_center_freq)(void* device);
typedef int(*rtlsdr_set_freq_correction)(void* device, int ppm);
typedef int(*rtlsdr_get_freq_correction)(void* device);
typedef int(*rtlsdr_get_tuner_gains)(void* devoce, int* gains);
typedef int(*rtlsdr_set_tuner_gain)(void* device, int gain);
typedef int(*rtlsdr_set_tuner_bandwidth)(void* device, unsigned int bandwidth);
typedef int(*rtlsdr_get_tuner_gain)(void* device);
typedef int(*rtlsdr_set_tuner_gain_mode)(void* device, int manual);
typedef int(*rtlsdr_set_sample_rate)(void* device, unsigned int rate);
typedef unsigned int(*rtlsdr_get_sample_rate)(void* device);
typedef int(*rtlsdr_set_agc_mode)(void* device, int on);
typedef int(*rtlsdr_reset_buffer)(void *device);
typedef int(*rtlsdr_read_sync)(void *device, void *buffer, int length, int *n_read);

class RtlSdrDllLoader
{
    HMODULE dllHandle;

public:
    // All the function pointers we need to access bytes from the device.
    rtlsdr_get_device_count get_device_count;
    rtlsdr_get_device_name get_device_name;
    rtlsdr_get_device_usb_strings get_device_usb_strings;
    rtlsdr_open open;
    rtlsdr_close close;
    rtlsdr_set_center_freq set_center_freq;
    rtlsdr_get_center_freq get_center_freq;
    rtlsdr_set_freq_correction set_freq_correction;
    rtlsdr_get_freq_correction get_freq_correction;
    rtlsdr_get_tuner_gains get_tuner_gains;
    rtlsdr_set_tuner_gain set_tuner_gain;
    rtlsdr_set_tuner_bandwidth set_tuner_bandwidth;
    rtlsdr_get_tuner_gain get_tuner_gain;
    rtlsdr_set_tuner_gain_mode set_tuner_gain_mode;
    rtlsdr_set_sample_rate set_sample_rate;
    rtlsdr_get_sample_rate get_sample_rate;
    rtlsdr_set_agc_mode set_agc_mode;
    rtlsdr_reset_buffer reset_buffer;
    rtlsdr_read_sync read_sync;

    RtlSdrDllLoader();
    bool Initialize();
    ~RtlSdrDllLoader();
};
