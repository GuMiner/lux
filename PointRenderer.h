#pragma once
#include <GL/glew.h>
#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include "GuCommon\shaders\ShaderFactory.h"
#include "GuCommon\vertex\PositionVbo.hpp"
#include "GuCommon\vertex\ColorVbo.hpp"

struct PointRendererProgram
{
    GLuint programId;

    GLuint projMatrixLocation;
    GLuint pointSizeLocation;
    GLuint transparencyLocation;
};

// Renders points.
class PointRenderer
{
    static PointRendererProgram glslProgram;

    GLuint vao;
    std::size_t lastBufferSize;

public:
    static bool LoadProgram(ShaderFactory* shaderFactory);

    PositionVbo positionBuffer;
    ColorVbo colorBuffer;

    PointRenderer();

    // Updates the GPU with whatever is currently in the point buffers.
    void Update();

    // Renders the points in the provided buffers.
    void Render(glm::mat4& projectionMatrix);

    // Helper methods for dealing with points.
    void Clear();


    ~PointRenderer();
};

