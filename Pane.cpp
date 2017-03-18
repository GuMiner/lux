#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm\gtc\matrix_transform.hpp>
#include "logging\Logger.h"
#include "Input.h"
#include "Pane.h"

Pane::Pane(glm::vec2 position, glm::vec2 size, Viewer* viewer, SentenceManager* sentenceManager, IPaneRenderable* paneContents)
    : position(position), size(size), viewer(viewer), sentenceManager(sentenceManager), paneContents(paneContents), borderRenderer(),
      titleOffset(0.40f), dragging(false)
{
    titleSentenceId = sentenceManager->CreateNewSentence();

    paneColor = glm::vec3(0.145f, 0.758f, 0.344f);
    sentenceManager->UpdateSentence(titleSentenceId, paneContents->GetTitle(), 16, paneColor);

    UpdatePaneBorders();
}

void Pane::UpdatePaneBorders()
{
    glm::vec2 sentenceDimensions = sentenceManager->GetSentenceDimensions(titleSentenceId);

    borderRenderer.Clear();
    borderRenderer.AddXYRectangle(glm::vec3(position.x, position.y, 0.0f), size, paneColor);
    borderRenderer.AddXYRectangle(glm::vec3(position.x, position.y + size.y, 0.0f), glm::vec2(sentenceDimensions.x, sentenceDimensions.y + titleOffset), paneColor);
    borderRenderer.Update();
}

bool Pane::HasClickedTitle()
{
    glm::vec2 sentenceDimensions = sentenceManager->GetSentenceDimensions(titleSentenceId);

    glm::vec2 gridPos = viewer->GetGridPos(Input::GetMousePos());
    return (gridPos.x > position.x && gridPos.x < position.x + sentenceDimensions.x &&
            gridPos.y > position.y + size.y && gridPos.y < position.y + size.y + sentenceDimensions.y + titleOffset);
}

void Pane::Update(float elapsedTime, float frameTime)
{
    if (paneContents->HasTitleUpdate())
    {
        sentenceManager->UpdateSentence(titleSentenceId, paneContents->GetTitle());
        UpdatePaneBorders();
    }

    // TODO manage input to move the pane around the screen.
    if (dragging)
    {
        position = viewer->GetGridPos(Input::GetMousePos()) - clickOffset;
        if (!Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            dragging = false;
        }

        Logger::Log("Position now: ", position);
        UpdatePaneBorders();
    }
    else if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && HasClickedTitle())
    {
        dragging = true;
        clickOffset = viewer->GetGridPos(Input::GetMousePos()) - position;
        Logger::Log("Started moving pane at position", position);
    }

    paneContents->Update(elapsedTime, frameTime);
}

void Pane::Render(glm::mat4& projectionMatrix, glm::mat4& perspectiveMatrix, glm::mat4& viewMatrix)
{
    borderRenderer.Render(projectionMatrix);

    float scaleDown = 0.50f;
    glm::mat4 titleSentenceMatrix = glm::translate(glm::mat4(), glm::vec3(position.x, position.y + size.y + titleOffset, 0.0f)) * viewMatrix;
    sentenceManager->RenderSentence(titleSentenceId, perspectiveMatrix, titleSentenceMatrix);

    float borderSize = viewer->GetUnitsPerPixel();
    paneContents->Render(projectionMatrix, position + glm::vec2(borderSize, borderSize), size - glm::vec2(borderSize * 2, borderSize * 2));
}
