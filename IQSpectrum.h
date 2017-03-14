#pragma once
#include "FilterBase.h"
#include "GuCommon\vertex\PositionVbo.hpp"
#include "GuCommon\vertex\ColorVbo.hpp"

struct PointSpectrumProgram
{
    GLuint programId;

    GLuint projMatrixLocation;
    GLuint pointSizeLocation;
    GLuint transparencyLocation;
};

struct PointSpectrumData
{
    GLuint vao;
    
    PositionVbo positionBuffer;
    ColorVbo colorBuffer;
    std::size_t lastBufferSize;
};

class IQSpectrum : public FilterBase
{
    PointSpectrumProgram spectrumProgram;
    PointSpectrumData spectrumData;

    std::mutex graphicsUpdateLock;

public:
    IQSpectrum(SdrBuffer* dataBuffer);
    virtual ~IQSpectrum();

    // Inherited via FilterBase
    virtual const char* GetName() const override
    {
        return "IQ Spectrum";
    }

    virtual bool LoadGraphics(ShaderFactory* shaderFactory) override;
    virtual void Process(std::vector<unsigned char>* block) override;
    virtual void Render(glm::mat4 & projectionMatrix) override;
};

