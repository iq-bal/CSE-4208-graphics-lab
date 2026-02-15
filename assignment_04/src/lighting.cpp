#include "lighting.h"

// Initialize Global State
bool dirLightOn = true;
bool pointLightOn = true;
bool spotLightOn = true;
bool ambientOn = true;
bool diffuseOn = true;
bool specularOn = true;
bool emissiveOn = true;

DirLight dirLight;
std::vector<PointLight> pointLights;
SpotLight spotLight;

void InitLights() {
  // 1. Directional Light (Sun)
  dirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
  dirLight.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
  dirLight.diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
  dirLight.specular = glm::vec3(0.8f, 0.8f, 0.8f);
  dirLight.enabled = true;

  // 2. Point Lights (Interior Ceiling - 6 lights)
  // Bus length is roughly -2.5 to 2.5 on Z, 1.0 wide on X.
  // Place lights along the ceiling (y ~ 0.6)
  float zStep = 4.0f / 5.0f; // Spread 6 lights over range
  float zStart = -2.0f;

  for (int i = 0; i < 6; i++) {
    PointLight light;
    light.position = glm::vec3(0.0f, 0.6f, zStart + i * zStep);

    light.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
    light.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    light.specular = glm::vec3(1.0f, 1.0f, 1.0f);

    light.constant = 1.0f;
    light.linear = 0.09f;
    light.quadratic = 0.032f;
    light.enabled = true;

    pointLights.push_back(light);
  }

  // 3. Spot Light (Headlight / Streetlight)
  spotLight.position = glm::vec3(0.0f, 2.0f, 5.0f);    // Above and front
  spotLight.direction = glm::vec3(0.0f, -0.5f, -1.0f); // Pointing down at bus
  spotLight.ambient = glm::vec3(0.0f, 0.0f, 0.0f);
  spotLight.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
  spotLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
  spotLight.constant = 1.0f;
  spotLight.linear = 0.09f;
  spotLight.quadratic = 0.032f;
  spotLight.cutOff = glm::cos(glm::radians(25.5f));
  spotLight.outerCutOff = glm::cos(glm::radians(30.5f)); // Soft edge
  spotLight.enabled = true;
}
