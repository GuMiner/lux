#include <algorithm>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm\gtc\matrix_transform.hpp>
#include "logging\Logger.h"
#include "Input.h"
#include "Pane.h"

glm::vec2 Pane::MinSize = glm::vec2(1.0f, 1.0f);

Pane::Pane(glm::vec2 position, glm::vec2 size, Viewer* viewer, SentenceManager* sentenceManager, IPaneRenderable* paneContents)
    : position(position), size(size), viewer(viewer), sentenceManager(sentenceManager), paneContents(paneContents), borderRenderer(),
      titleOffset(0.40f), dragging(false), resizing(false)
{
    titleSentenceId = sentenceManager->CreateNewSentence();

    paneColor = glm::vec3(0.145f, 0.758f, 0.344f);
    sentenceManager->UpdateSentence(titleSentenceId, paneContents->GetTitle(), 14, paneColor);

    UpdatePaneStructure();
}

void Pane::UpdatePaneStructure()
{
    glm::vec2 sentenceDimensions = sentenceManager->GetSentenceDimensions(titleSentenceId);

    borderRenderer.Clear();

    // border
    borderRenderer.AddXYRectangle(glm::vec3(position.x, position.y, 0.0f), size, paneColor);

    // sentence callout
    borderRenderer.AddXYRectangle(glm::vec3(position.x, position.y + size.y, 0.0f), glm::vec2(sentenceDimensions.x, sentenceDimensions.y + titleOffset), paneColor);

    // resize
    borderRenderer.positionBuffer.vertices.push_back(glm::vec3(position.x + size.x - Pane::MinSize.x, position.y, 0.0f));
    borderRenderer.colorBuffer.vertices.push_back(paneColor);

    borderRenderer.positionBuffer.vertices.push_back(glm::vec3(position.x + size.x, position.y + Pane::MinSize.y, 0.0f));
    borderRenderer.colorBuffer.vertices.push_back(paneColor);

    borderRenderer.positionBuffer.vertices.push_back(glm::vec3(position.x + size.x - Pane::MinSize.x * 0.66f, position.y, 0.0f));
    borderRenderer.colorBuffer.vertices.push_back(paneColor);

    borderRenderer.positionBuffer.vertices.push_back(glm::vec3(position.x + size.x, position.y + Pane::MinSize.y * 0.66f, 0.0f));
    borderRenderer.colorBuffer.vertices.push_back(paneColor);

    borderRenderer.positionBuffer.vertices.push_back(glm::vec3(position.x + size.x - Pane::MinSize.x * 0.33f, position.y, 0.0f));
    borderRenderer.colorBuffer.vertices.push_back(paneColor);

    borderRenderer.positionBuffer.vertices.push_back(glm::vec3(position.x + size.x, position.y + Pane::MinSize.y * 0.33f, 0.0f));
    borderRenderer.colorBuffer.vertices.push_back(paneColor);

    borderRenderer.Update();
}

bool Pane::HasClickedTitle()
{
    glm::vec2 sentenceDimensions = sentenceManager->GetSentenceDimensions(titleSentenceId);

    glm::vec2 gridPos = viewer->GetGridPos(Input::GetMousePos());
    return (gridPos.x > position.x && gridPos.x < position.x + sentenceDimensions.x &&
            gridPos.y > position.y + size.y && gridPos.y < position.y + size.y + sentenceDimensions.y + titleOffset);
}

bool Pane::HasClickedResizer()
{
    glm::vec2 gridPos = viewer->GetGridPos(Input::GetMousePos());
    return (gridPos.x > position.x + size.x - Pane::MinSize.x && gridPos.x < position.x + size.x &&
        gridPos.y > position.y && gridPos.y < position.y + Pane::MinSize.y);
}

void Pane::Update(float elapsedTime, float frameTime)
{
    if (paneContents->HasTitleUpdate())
    {
        sentenceManager->UpdateSentence(titleSentenceId, paneContents->GetTitle());
        UpdatePaneStructure();
    }

    // TODO manage input to move the pane around the screen.
    if (dragging)
    {
        position = viewer->GetGridPos(Input::GetMousePos()) - clickOffset;
        if (!Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            dragging = false;
        }

        UpdatePaneStructure();
    }
    else if (resizing)
    {
        glm::vec2 newMousePos = viewer->GetGridPos(Input::GetMousePos()) - clickOffset;
        size.x = newMousePos.x - position.x;

        size.y -= (newMousePos.y - position.y);
        position.y = newMousePos.y;

        size.x = std::max(Pane::MinSize.x, size.x);
        size.y = std::max(Pane::MinSize.y, size.y);

        if (!Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            resizing = false;
        }

        UpdatePaneStructure();
    }
    else if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
    {
        if (HasClickedTitle())
        {
            dragging = true;
            clickOffset = viewer->GetGridPos(Input::GetMousePos()) - position;
        }
        else if (HasClickedResizer())
        {
            resizing = true;
            clickOffset = viewer->GetGridPos(Input::GetMousePos()) - (position + glm::vec2(size.x, 0.0f));
        }
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
