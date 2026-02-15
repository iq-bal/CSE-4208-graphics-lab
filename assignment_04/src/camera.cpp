#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 customLookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
  // 1. Calculate the camera direction vector (z-axis)
  // Note: In OpenGL, the camera looks towards -z, so z-axis is (camera -
  // target)
  glm::vec3 zaxis = glm::normalize(eye - center);

  // 2. Calculate the right axis (x-axis)
  // Cross product of World Up and Camera Z
  glm::vec3 xaxis = glm::normalize(glm::cross(up, zaxis));

  // 3. Calculate the camera up axis (y-axis)
  // Cross product of Camera Z and Camera X
  glm::vec3 yaxis = glm::cross(zaxis, xaxis);

  // 4. Create the View Matrix
  // The view matrix is composed of a Rotation matrix and a Translation matrix
  // View = Rotation * Translation

  // Translation Matrix: Moves the world so the camera is at (0,0,0)
  glm::mat4 translation(1.0f);
  translation[3][0] = -eye.x;
  translation[3][1] = -eye.y;
  translation[3][2] = -eye.z;

  // Rotation Matrix: Aligns the world axes with the camera axes
  // First row: xaxis
  // Second row: yaxis
  // Third row: zaxis
  glm::mat4 rotation(1.0f);
  rotation[0][0] = xaxis.x;
  rotation[1][0] = xaxis.y;
  rotation[2][0] = xaxis.z;

  rotation[0][1] = yaxis.x;
  rotation[1][1] = yaxis.y;
  rotation[2][1] = yaxis.z;

  rotation[0][2] = zaxis.x;
  rotation[1][2] = zaxis.y;
  rotation[2][2] = zaxis.z;

  // View Matrix = Rotation * Translation
  return rotation * translation;
}
