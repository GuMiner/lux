#include "GraphicsSetup.h"
#include <glm/gtc/matrix_transform.hpp>

float FOV_Y = 50.0f;

float GraphicsSetup::AspectRatio = 1.77778f;
float GraphicsSetup::NearPlane = 0.10f;
float GraphicsSetup::FarPlane = 1000.0f;
glm::mat4 GraphicsSetup::PerspectiveMatrix = glm::perspective(glm::radians(FOV_Y), GraphicsSetup::AspectRatio, GraphicsSetup::NearPlane, GraphicsSetup::FarPlane);

int GraphicsSetup::ScreenWidth = 1280;
int GraphicsSetup::ScreenHeight = 720;
int GraphicsSetup::MaxFramerate = 120;