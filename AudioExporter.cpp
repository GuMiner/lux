#include <SFML\Audio.hpp>
#include "AudioExporter.h"


AudioExporter::AudioExporter(SdrBuffer* sdrBuffer)
    : amAudioTransformer()
{
    audioStream = new AudioStream(sdrBuffer, &amAudioTransformer);
    audioStream->Start();
}

AudioExporter::~AudioExporter()
{
    audioStream->Stop();
    delete audioStream;
}
