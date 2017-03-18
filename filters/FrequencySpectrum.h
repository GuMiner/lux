#pragma once
#include "FilterBase.h"
#include "GuCommon\shaders\ShaderFactory.h"
#include "math\WindowedSincFilter.h"
#include "IPaneRenderable.h"
#include "PointRenderer.h"
#include "LineRenderer.h"

class FrequencySpectrum : public FilterBase, public IPaneRenderable
{
    PointRenderer fftReals;
    PointRenderer fftImags;

    glm::vec2 lastPosition;
    glm::vec2 lastSize;
    bool updateGraphics;
    std::mutex graphicsUpdateLock;

public:
    FrequencySpectrum(glm::vec2 startPosition, glm::vec2 startSize, SdrBuffer* dataBuffer);
    virtual ~FrequencySpectrum();

    // Inherited via FilterBase
    virtual std::string GetName() const override;
    virtual void Process(std::vector<unsigned char>* block) override;

    // Inherited via IPaneRenderable
    virtual bool HasTitleUpdate() override;
    virtual std::string GetTitle() override;
    virtual void Update(float elapsedTime, float frameTime) override;
    virtual void Render(glm::mat4 & projectionMatrix, glm::vec2 position, glm::vec2 size) override;
};

