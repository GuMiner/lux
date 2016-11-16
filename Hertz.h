#pragma once
#include <string>

// Makes it easier to deal with units of frequency.
class Hertz
{
    unsigned int frequency; // In Hertz

public:
    Hertz(unsigned int hz);
    Hertz(unsigned int kHz, unsigned int hz);
    Hertz(unsigned int MHz, unsigned int kHz, unsigned int hz);
    Hertz(unsigned int GHz, unsigned int MHz, unsigned int kHz, unsigned int hz);
    std::string ToFriendlyString();
    unsigned int GetFrequency() const;
};

