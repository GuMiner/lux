#pragma once
#include <string>
#include <GL/glew.h>
#include "FilterBase.h"
#include "GuCommon\vertex\PositionVbo.hpp"
#include "GuCommon\vertex\ColorVbo.hpp"
#include "math\WindowedSincFilter.h"
#include "IPaneRenderable.h"
#include "PointRenderer.h"

class IQSpectrum : public FilterBase, public IPaneRenderable
{
    PointRenderer iqPoints;

    glm::vec2 lastPosition;
    glm::vec2 lastSize;
    bool updateGraphics;
    std::mutex graphicsUpdateLock;

    WindowedSincFilter windowedSincFilter;
    void FormDecimator();

public:
    IQSpectrum(glm::vec2 startPosition, glm::vec2 startSize, SdrBuffer* dataBuffer);
    virtual ~IQSpectrum();

    // Inherited via FilterBase
    virtual std::string GetName() const override;
    virtual void Process(std::vector<unsigned char>* block) override;

    // Inherited via IPaneRenderable
    virtual bool HasTitleUpdate() override;
    virtual std::string GetTitle() override;
    virtual void Update(float elapsedTime, float frameTime) override;
    virtual void Render(glm::mat4 & projectionMatrix, glm::vec2 position, glm::vec2 size) override;
};

