#pragma once
#include "AudioStream.h"
#include "AMAudioTransformer.h"

// Exports provided sample data as audio by piping it into the current default output audio device.
class AudioExporter
{
    AudioStream* audioStream;
    AMAudioTransformer amAudioTransformer;

public:
    AudioExporter(SdrBuffer* sdrBuffer);
    virtual ~AudioExporter();
};

