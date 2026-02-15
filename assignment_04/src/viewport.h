#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "shader.h"
#include <glm/glm.hpp>

enum CameraMode { CAM_ISO, CAM_TOP, CAM_FRONT, CAM_INSIDE };

struct ViewportState {
  CameraMode mode;
  int x, y, width, height;
};

// Global Viewports
extern ViewportState viewports[4];

void InitViewports(int width, int height);
void SetupViewportLighting(Shader &shader, int viewportIndex);
glm::mat4 GetViewportViewMatrix(int viewportIndex, glm::vec3 busPos,
                                float busYaw);

#endif
