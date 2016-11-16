#include <sstream>
#include "Hertz.h"

Hertz::Hertz(unsigned int hz)
    : frequency(hz)
{ }

Hertz::Hertz(unsigned int kHz, unsigned int hz)
    : Hertz(kHz * 1000 + hz)
{ }

Hertz::Hertz(unsigned int MHz, unsigned int kHz, unsigned int hz)
    : Hertz(MHz * 1000 + kHz, hz)
{ }
Hertz::Hertz(unsigned int GHz, unsigned int MHz, unsigned int kHz, unsigned int hz)
    : Hertz(GHz * 1000 + MHz, kHz, hz)
{ }

std::string Hertz::ToFriendlyString()
{
    const unsigned int kHzScale = 1000;
    const unsigned int MHzScale = 1000 * kHzScale;
    const unsigned int GHzScale = 1000 * MHzScale;
    

    unsigned int GHz = frequency / GHzScale;
    unsigned int MHz = (frequency - GHz * GHzScale) / MHzScale;
    unsigned int kHz = (frequency - (GHz * GHzScale + MHz * MHzScale)) / kHzScale;
    unsigned int hz = frequency - (GHz * GHzScale + MHz * MHzScale * kHz * kHzScale);

    bool allMinorScales = false;
    std::stringstream frequencyString;
    if (GHz != 0)
    {
        frequencyString << GHz << " GHz, ";
        allMinorScales = true;
    }

    if (MHz != 0 || allMinorScales)
    {
        frequencyString << MHz << " MHz, ";
        allMinorScales = true;
    }

    if (kHz != 0 || allMinorScales)
    {
        frequencyString << kHz << " kHz, ";
    }

    frequencyString << hz << " Hz.";
    return frequencyString.str();
}

unsigned int Hertz::GetFrequency() const
{
    return frequency;
}