#include <glm\gtc\matrix_transform.hpp>
#include "Pane.h"

Pane::Pane(glm::vec2 position, glm::vec2 size, Viewer* viewer, SentenceManager* sentenceManager, IPaneRenderable* paneContents)
    : position(position), size(size), viewer(viewer), sentenceManager(sentenceManager), paneContents(paneContents), borderRenderer()
{
    titleSentenceId = sentenceManager->CreateNewSentence();
    paneColor = glm::vec3(0.145f, 0.758f, 0.344f);
    // sentenceManager->UpdateSentence(titleSentenceId, paneContents->GetTitle(), 22, paneColor);

    borderRenderer.Clear();
    borderRenderer.AddXYRectangle(glm::vec3(position.x, position.y, 0.0f), size, paneColor);
    borderRenderer.Update();
}

void Pane::Update(float elapsedTime, float frameTime)
{
    if (paneContents->HasTitleUpdate())
    {
        // Get and update title.
        sentenceManager->UpdateSentence(titleSentenceId, paneContents->GetTitle(), 22, paneColor);
    }

    // TODO manage input to move the pane around the screen.

    paneContents->Update(elapsedTime, frameTime);
}

void Pane::Render(glm::mat4& projectionMatrix, glm::mat4& perspectiveMatrix, glm::mat4& viewMatrix)
{
    borderRenderer.Render(projectionMatrix);

    glm::mat4 mouseToolTipMatrix = glm::scale(glm::mat4(), glm::vec3(0.042f, 0.042f, 0.042f)) * glm::translate(glm::mat4(), glm::vec3(position.x, position.y + size.y, 0.0f)) * viewMatrix;
    sentenceManager->RenderSentence(titleSentenceId, perspectiveMatrix, mouseToolTipMatrix);

    float borderSize = viewer->GetUnitsPerPixel();
    paneContents->Render(projectionMatrix, position + glm::vec2(borderSize, borderSize), size - glm::vec2(borderSize * 2, borderSize * 2));
}
