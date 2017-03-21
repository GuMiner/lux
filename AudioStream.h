#pragma once
#include <SFML\Audio.hpp>
#include <vector>
#include <mutex>
#include "filters\FilterBase.h"
#include "sdr\SdrBuffer.h"
#include "IAudioTransformer.h"

class AudioStream : public sf::SoundStream, public FilterBase
{
    const unsigned int Channels = 2;
    const unsigned int SampleRate = 44100;

    std::mutex audioCopyMutex;
    bool audioOnFirstBuffer;
    std::vector<sf::Int16> pingPongFirstBuffer;
    std::vector<sf::Int16> pingPongSecondBuffer;

    IAudioTransformer* audioTransformer;

public:
    AudioStream(SdrBuffer* sdrBuffer, IAudioTransformer* initialAudioTransformer);
    virtual ~AudioStream();

    // Starts / Stops the audio stream.
    void Start();
    void Stop();

    // Inherited via SoundStream
    virtual bool onGetData(Chunk & data) override;
    virtual void onSeek(sf::Time timeOffset) override;

    // Inherited via FilterBase
    virtual std::string GetName() const override;
    virtual void Process(std::vector<unsigned char>* block) override;

    void SetAudioTransformer(IAudioTransformer* audioTransformer);
};

