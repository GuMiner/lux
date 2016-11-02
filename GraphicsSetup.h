#pragma once
#include <glm\mat4x4.hpp>

struct GraphicsSetup
{
    static float AspectRatio;
    static float NearPlane;
    static float FarPlane;
    static glm::mat4 PerspectiveMatrix;

    static int ScreenWidth;
    static int ScreenHeight;
    static int MaxFramerate;
};

