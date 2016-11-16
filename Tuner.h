#pragma once
#include "Hertz.h"
class Tuner
{
    Hertz currentFrequency;

public:
    Tuner();
    bool TuneTo(Hertz frequency);
    ~Tuner();
};

