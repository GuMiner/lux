#include "AudioStream.h"

AudioStream::AudioStream(SdrBuffer* sdrBuffer)
    : audioOnFirstBuffer(true), FilterBase(sdrBuffer)
{
    initialize(AudioStream::Channels, AudioStream::SampleRate);
}

void AudioStream::Start()
{
    enabled = true;
    play();
}

void AudioStream::Stop()
{
    enabled = false;
    stop();
}

AudioStream::~AudioStream()
{
    StopFilter();
}

bool AudioStream::onGetData(Chunk& data)
{
    audioCopyMutex.lock();
    data.samples = audioOnFirstBuffer ? &pingPongSecondBuffer[0] : &pingPongFirstBuffer[0];
    data.sampleCount = audioOnFirstBuffer ? pingPongSecondBuffer.size() : pingPongFirstBuffer.size();
    data.sampleCount /= AudioStream::Channels;
    audioCopyMutex.unlock();
    return true;
}

void AudioStream::onSeek(sf::Time timeOffset)
{
    // We don't support data seeking of SDR audio feeds.
}

std::string AudioStream::GetName() const
{
    return "Audio Filter";
}

void AudioStream::Process(std::vector<unsigned char>* block)
{
    // Configuration:
    // - AM modulation: downsample the data to our output sample rate, 
    //   then take the amplitude of the I/Q stream as audio samples.
    //   If we're downsampling to too low a value, upsample to the requested output rate.

    audioCopyMutex.lock();
    // Copy in whatever audio data we want
    // TODO should write computational modules that perform SDR data stream operations.
    // The FT and WS filters should probably be inside whatever structure holds the modules together,
    // along with what I write for FM and AM audio demodulation..
    audioCopyMutex.unlock();
}
