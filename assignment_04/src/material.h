#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

struct Material {
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  float shininess;
  glm::vec3 emissive; // For self-glowing objects
};

#endif
