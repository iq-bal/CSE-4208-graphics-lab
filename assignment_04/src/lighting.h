#ifndef LIGHTING_H
#define LIGHTING_H

#include <glm/glm.hpp>
#include <vector>

struct DirLight {
  glm::vec3 direction;
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  bool enabled;
};

struct PointLight {
  glm::vec3 position;

  float constant;
  float linear;
  float quadratic;

  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  bool enabled;
};

struct SpotLight {
  glm::vec3 position;
  glm::vec3 direction;
  float cutOff;
  float outerCutOff;

  float constant;
  float linear;
  float quadratic;

  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  bool enabled;
};

// Global Lighting State
extern bool dirLightOn;
extern bool pointLightOn;
extern bool spotLightOn;
extern bool ambientOn;
extern bool diffuseOn;
extern bool specularOn;
extern bool emissiveOn;

// Light Instances
extern DirLight dirLight;
extern std::vector<PointLight> pointLights;
extern SpotLight spotLight;

// Function to initialize default lights
void InitLights();

#endif
