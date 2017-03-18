#pragma once
#include <string>
#include <GL/glew.h>
#include "FilterBase.h"
#include "GuCommon\vertex\PositionVbo.hpp"
#include "GuCommon\vertex\ColorVbo.hpp"
#include "math\WindowedSincFilter.h"
#include "IPaneRenderable.h"
#include "LineRenderer.h"

class Spectrum : public FilterBase, public IPaneRenderable
{
    LineRenderer spectrumLines;

    glm::vec2 lastPosition;
    glm::vec2 lastSize;
    bool updateGraphics;
    std::mutex graphicsUpdateLock;

    WindowedSincFilter windowedSincFilter;
    void FormDecimator();

public:
    Spectrum(glm::vec2 startPosition, glm::vec2 startSize, SdrBuffer* dataBuffer);
    virtual ~Spectrum();

    // Inherited via FilterBase
    virtual std::string GetName() const override;
    virtual void Process(std::vector<unsigned char>* block) override;

    // Inherited via IPaneRenderable
    virtual bool HasTitleUpdate() override;
    virtual std::string GetTitle() override;
    virtual void Update(float elapsedTime, float frameTime) override;
    virtual void Render(glm::mat4 & projectionMatrix, glm::vec2 position, glm::vec2 size) override;
};

