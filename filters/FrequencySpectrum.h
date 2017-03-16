#pragma once
#include "FilterBase.h"
#include "GuCommon\vertex\PositionVbo.hpp"
#include "GuCommon\vertex\ColorVbo.hpp"
#include "math\WindowedSincFilter.h"

struct FreqPointSpectrumProgram
{
    GLuint programId;

    GLuint projMatrixLocation;
    GLuint pointSizeLocation;
    GLuint transparencyLocation;
};

struct FreqPointSpectrumData
{
    GLuint vao;
    
    PositionVbo positionBuffer;
    ColorVbo colorBuffer;
    std::size_t lastBufferSize;
};

struct FreqBorderData
{
    GLuint vao;
    PositionVbo positionBuffer;
    ColorVbo colorBuffer;
};

class FrequencySpectrum : public FilterBase
{
    FreqPointSpectrumProgram spectrumProgram;
    FreqPointSpectrumData spectrumData;
    FreqBorderData borderData;
    
    std::mutex graphicsUpdateLock;
    bool firstUpdate;

public:
    FrequencySpectrum(SdrBuffer* dataBuffer);
    virtual ~FrequencySpectrum();

    // Inherited via FilterBase
    virtual const char* GetName() const override
    {
        return "Frequency Spectrum";
    }

    virtual bool LoadGraphics(ShaderFactory* shaderFactory) override;
    virtual void Process(std::vector<unsigned char>* block) override;
    virtual void Render(glm::mat4 & projectionMatrix) override;
};

