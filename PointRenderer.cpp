#include "logging\Logger.h"
#include "PointRenderer.h"

PointRendererProgram PointRenderer::glslProgram;

bool PointRenderer::LoadProgram(ShaderFactory* shaderFactory)
{
    Logger::Log("Loading the point rendering shading program for point rendering...");
    if (!shaderFactory->CreateShaderProgram("pointRender", &glslProgram.programId))
    {
        Logger::LogError("Failed to load the point rendering shader; cannot continue.");
        return false;
    }

    glslProgram.projMatrixLocation = glGetUniformLocation(glslProgram.programId, "projMatrix");
    glslProgram.pointSizeLocation = glGetUniformLocation(glslProgram.programId, "pointSize");
    glslProgram.transparencyLocation = glGetUniformLocation(glslProgram.programId, "transparency");

    return true;
}

PointRenderer::PointRenderer()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    positionBuffer.Initialize();
    colorBuffer.Initialize();
    lastBufferSize = 0;
}

void PointRenderer::Update()
{
    glBindVertexArray(vao);
    positionBuffer.TransferToOpenGl();
    colorBuffer.TransferToOpenGl();
    lastBufferSize = positionBuffer.vertices.size();
}


void PointRenderer::Render(glm::mat4& projectionMatrix)
{
    glUseProgram(glslProgram.programId);
    glBindVertexArray(vao);
    glUniformMatrix4fv(glslProgram.projMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

    // TODO these should be configurable.
    glUniform1f(glslProgram.transparencyLocation, 1.0f);
    glUniform1f(glslProgram.pointSizeLocation, 1.0f);

    glDrawArrays(GL_POINTS, 0, lastBufferSize);
}

void PointRenderer::Clear()
{
    positionBuffer.vertices.clear();
    colorBuffer.vertices.clear();
}

PointRenderer::~PointRenderer()
{
    glDeleteVertexArrays(1, &vao);
    positionBuffer.Deinitialize();
    colorBuffer.Deinitialize();
}
