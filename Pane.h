#pragma once
#include <glm\mat4x4.hpp>
#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include "text\SentenceManager.h"
#include "IPaneRenderable.h"
#include "LineRenderer.h"
#include "Viewer.h"

class Pane
{
    Viewer* viewer;
    SentenceManager* sentenceManager;

    glm::vec2 position;
    glm::vec2 size;

    IPaneRenderable* paneContents;

    // GUI of the pane
    float titleOffset;
    int titleSentenceId;
    glm::vec3 paneColor;
    LineRenderer borderRenderer;

    void UpdatePaneBorders();

    bool dragging;
    glm::vec2 clickOffset; // The offset the user clicked the title in comparison to where the pane is.
    bool HasClickedTitle();

public:
    Pane(glm::vec2 position, glm::vec2 size, Viewer* viewer, SentenceManager* sentenceManager, IPaneRenderable* paneContents);

    void Update(float elapsedTime, float frameTime);
    void Render(glm::mat4& projectionMatrix, glm::mat4& perspectiveMatrix, glm::mat4& viewMatrix);
};

