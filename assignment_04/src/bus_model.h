#ifndef BUS_MODEL_H
#define BUS_MODEL_H

#include "cube.h"
#include "shader.h"
#include <glm/glm.hpp>

// Complex model composed of multiple primitives
class BusModel {
public:
  BusModel();
  void Draw(Shader &shader, glm::mat4 modelMatrix);

private:
  Cube bodyCube;
  Sphere wheelSphere;

  // Helper to draw specific parts
  void DrawBody(Shader &shader, glm::mat4 model);
  void DrawWheels(Shader &shader, glm::mat4 model);
  void DrawInterior(Shader &shader, glm::mat4 model);
  void DrawWindows(Shader &shader, glm::mat4 model);
};

#endif
