#pragma once
#include "AudioStream.h"

// Exports provided sample data as audio by piping it into the current default output audio device.
class AudioExporter
{
    AudioStream* audioStream;
    
public:
    AudioExporter(AudioStream* audioStream);
    ~AudioExporter();
};

