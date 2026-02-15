#include "viewport.h"
#include "camera.h"
#include "lighting.h"

ViewportState viewports[4];

void InitViewports(int width, int height) {
  int halfW = width / 2;
  int halfH = height / 2;

  // TL: Combined
  viewports[0] = {CAM_ISO, 0, halfH, halfW, halfH};
  // TR: Ambient Only
  viewports[1] = {CAM_TOP, halfW, halfH, halfW, halfH};
  // BL: Diffuse Only
  viewports[2] = {CAM_FRONT, 0, 0, halfW, halfH};
  // BR: Directional Only
  viewports[3] = {CAM_INSIDE, halfW, 0, halfW, halfH};
}

void SetupViewportLighting(Shader &shader, int i) {
  // Defaults: Use global user settings
  bool d = dirLightOn;
  bool p = pointLightOn;
  bool s = spotLightOn;
  bool amb = ambientOn;
  bool diff = diffuseOn;
  bool spec = specularOn;
  bool emit = emissiveOn;

  // Overrides per Viewport Spec
  if (i == 0) {
    // TL: Combined (All ON if user says so, no override)
  } else if (i == 1) {
    // TR: Ambient Only (Disable all else)
    d = true;
    p = false;
    s = false; // Only Dir light source for Ambient base? Or global?
    // Actually, enable light sources but DISABLE Diffuse/Spec components.
    // Assuming Ambient requires at least one light source enabled to calculate
    // "light.ambient * mat.ambient" Or if we interpret "Ambient Only" as "Only
    // Global Ambient"? Shader uses `light.ambient`. So yes, keep light source
    // enabled.
    amb = true;
    diff = false;
    spec = false;
    emit = false;
  } else if (i == 2) {
    // BL: Diffuse Only
    amb = false;
    diff = true;
    spec = false;
    emit = false;
  } else if (i == 3) {
    // BR: Directional Only
    // Meaning: Only Directional Light Source Active
    d = true;
    p = false;
    s = false;

    // And usually full components for that light?
    // "Directional Only" usually refers to the Light *Source*.
    // So allow amb/diff/spec for that source.
    // But strict interpretation of table:
    // TL=Combined, TR=Amb, BL=Diff, BR=Directional.
    // If BR means "Directional Light Source", then we follow global component
    // switches? Or does it mean "Only Directional Light logic"? Let's assume BR
    // is "Only Directional Light Source", but Components follow Global or
    // Default? Let's assume user wants to see the Directional Light's effect.
    amb = false; // Usually directional light shadows are black without ambient.
    diff = true;
    spec = true;
    emit = false;
  }

  shader.setBool("dirLightOn", d);
  shader.setBool("pointLightOn", p);
  shader.setBool("spotLightOn", s);
  shader.setBool("ambientOn", amb);
  shader.setBool("diffuseOn", diff);
  shader.setBool("specularOn", spec);
  shader.setBool("emissiveOn", emit);
}

glm::mat4 GetViewportViewMatrix(int i, glm::vec3 busPos, float busYaw) {
  CameraMode mode = viewports[i].mode;

  if (mode == CAM_ISO) {
    glm::vec3 pos = busPos + glm::vec3(-8.0f, 8.0f, 8.0f);
    return customLookAt(pos, busPos, glm::vec3(0, 1, 0));
  } else if (mode == CAM_TOP) {
    glm::vec3 pos = busPos + glm::vec3(0.0f, 15.0f, 0.1f);
    return customLookAt(pos, busPos, glm::vec3(0, 0, -1)); // Top down
  } else if (mode == CAM_FRONT) {
    glm::vec3 pos = busPos + glm::vec3(0.0f, 2.0f, 10.0f);
    return customLookAt(pos, busPos, glm::vec3(0, 1, 0));
  } else if (mode == CAM_INSIDE) {
    // Driver position
    glm::vec3 pos = busPos + glm::vec3(0.5f, 0.5f, 2.0f);
    // Forward vector
    glm::vec3 target;
    target.x = pos.x + sin(glm::radians(busYaw));
    target.z = pos.z + cos(glm::radians(busYaw));
    target.y = pos.y; // Look straight
    return customLookAt(pos, target, glm::vec3(0, 1, 0));
  }
  return glm::mat4(1.0f);
}
