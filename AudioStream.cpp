#include <limits>
#include "AudioStream.h"

AudioStream::AudioStream(SdrBuffer* sdrBuffer, IAudioTransformer* initialAudioTransformer)
    : audioOnFirstBuffer(true), audioTransformer(initialAudioTransformer), FilterBase(sdrBuffer)
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
    data.sampleCount = audioOnFirstBuffer ? pingPongFirstBuffer.size() : pingPongSecondBuffer.size();
    data.sampleCount /= AudioStream::Channels;

    if (data.sampleCount == 0)
    {
        data.sampleCount += 44100 * 2;
        if (audioOnFirstBuffer)
        {
            for (int i = 0; i < 44100 * 2; i++)
            {
                pingPongFirstBuffer.push_back(0);
                pingPongFirstBuffer.push_back(std::numeric_limits<signed short>::max());
                pingPongFirstBuffer.push_back(std::numeric_limits<signed short>::max());
                pingPongFirstBuffer.push_back(0);
            }
        }
        else
        {
            for (int i = 0; i < 44100 * 2; i++)
            {
                pingPongSecondBuffer.push_back(0);
                pingPongSecondBuffer.push_back(std::numeric_limits<signed short>::max());
                pingPongSecondBuffer.push_back(std::numeric_limits<signed short>::max());
                pingPongSecondBuffer.push_back(0);
            }
        }
    }


    data.samples = audioOnFirstBuffer ? &pingPongFirstBuffer[0] : &pingPongSecondBuffer[0];

    // Audio will be on the other buffer for the next iteration, so clear it for new samples.
    if (audioOnFirstBuffer)
    {
        pingPongSecondBuffer.clear();
    }
    else
    {
        pingPongFirstBuffer.clear();
    }

    audioOnFirstBuffer = !audioOnFirstBuffer; 
    Logger::Log("Playing from buffer ", audioOnFirstBuffer);

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
    audioCopyMutex.lock();
    Logger::Log("Processing samples ", block->size(), " ", pingPongFirstBuffer.size(), "-", pingPongSecondBuffer.size(), audioOnFirstBuffer);
    if (this->getStatus() != sf::SoundSource::Playing)
    {
        Logger::Log("Restarting play...");
        play();
    }
    audioTransformer->Process(block, audioOnFirstBuffer ? &pingPongFirstBuffer : &pingPongSecondBuffer); // Note that audio is currently playing on the *other* buffer
    audioCopyMutex.unlock();
}
