#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "bus_model.h"
#include "camera.h"
#include "cube.h" // For road rendering
#include "lighting.h"
#include "shader.h"
#include "viewport.h"

// Global State
float deltaTime = 0.0f;
float lastFrame = 0.0f;
glm::vec3 busPos(0.0f);
float busYaw = 0.0f;
float busSpeed = 5.0f;

// Input Callback
void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // Bus Controls
  float velocity = busSpeed * deltaTime;
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS &&
          glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
    busPos.x += sin(glm::radians(busYaw)) * velocity;
    busPos.z += cos(glm::radians(busYaw)) * velocity;
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    busPos.x -= sin(glm::radians(busYaw)) * velocity;
    busPos.z -= cos(glm::radians(busYaw)) * velocity;
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    busYaw += 50.0f * deltaTime;
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    busYaw -= 50.0f * deltaTime;
  }

  // Toggle Inputs (Debounced)
  static bool k1 = false, k2 = false, k3 = false, k5 = false, k6 = false,
              k7 = false;

  // Light Sources
  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !k1) {
    dirLightOn = !dirLightOn;
    k1 = true;
    std::cout << "Dir Light: " << dirLightOn << std::endl;
  }
  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE)
    k1 = false;

  if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !k2) {
    pointLightOn = !pointLightOn;
    k2 = true;
    std::cout << "Point Light: " << pointLightOn << std::endl;
  }
  if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE)
    k2 = false;

  if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !k3) {
    spotLightOn = !spotLightOn;
    k3 = true;
    std::cout << "Spot Light: " << spotLightOn << std::endl;
  }
  if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE)
    k3 = false;

  // Components
  if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS && !k5) {
    ambientOn = !ambientOn;
    k5 = true;
    std::cout << "Ambient: " << ambientOn << std::endl;
  }
  if (glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE)
    k5 = false;

  if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS && !k6) {
    diffuseOn = !diffuseOn;
    k6 = true;
    std::cout << "Diffuse: " << diffuseOn << std::endl;
  }
  if (glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE)
    k6 = false;

  if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS && !k7) {
    specularOn = !specularOn;
    k7 = true;
    std::cout << "Specular: " << specularOn << std::endl;
  }
  if (glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE)
    k7 = false;

  // Camera Cycling per Viewport
  static bool kQ = false, kW = false, kE = false, kR = false;
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && !kQ) {
    viewports[0].mode = static_cast<CameraMode>((viewports[0].mode + 1) % 4);
    kQ = true;
  } else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE)
    kQ = false;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS &&
      glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !kW) {
    viewports[1].mode = static_cast<CameraMode>((viewports[1].mode + 1) % 4);
    kW = true;
  } else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE)
    kW = false;

  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !kE) {
    viewports[2].mode = static_cast<CameraMode>((viewports[2].mode + 1) % 4);
    kE = true;
  } else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE)
    kE = false;

  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !kR) {
    viewports[3].mode = static_cast<CameraMode>((viewports[3].mode + 1) % 4);
    kR = true;
  } else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
    kR = false;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  InitViewports(width, height);
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window =
      glfwCreateWindow(1000, 800, "Assignment 04 - Multi-Viewport", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  if (glewInit() != GLEW_OK) {
    std::cout << "Failed to initialize GLEW" << std::endl;
    return -1;
  }
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST); // Needed for viewport clearing

  // Initialize Systems
  Shader shader("shaders/lighting.vert", "shaders/lighting.frag");
  BusModel bus;
  Cube roadCube(glm::vec3(1.0f));
  InitLights();

  int w, h;
  glfwGetFramebufferSize(window, &w, &h);
  InitViewports(w, h); // Initial Viewport Setup

  // Main Loop
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    // Clear Background
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();

    // 1. Update Lighting Uniforms (Positions need to be dynamic for
    // SpotLight/PointLight if they moved,
    //    but PointLights are static on bus, so we must update their World
    //    Positions based on Bus Model Matrix)

    // Pass Directional Light (Fixed)
    shader.setVec3("dirLight.direction", dirLight.direction);
    shader.setVec3("dirLight.ambient", dirLight.ambient);
    shader.setVec3("dirLight.diffuse", dirLight.diffuse);
    shader.setVec3("dirLight.specular", dirLight.specular);

    // Pass Point Lights (Attached to Bus)
    glm::mat4 busModel = glm::mat4(1.0f);
    busModel = glm::translate(busModel, busPos);
    busModel = glm::rotate(busModel, glm::radians(busYaw), glm::vec3(0, 1, 0));

    for (int i = 0; i < pointLights.size(); i++) {
      // Transform local point light pos to world pos
      glm::vec4 worldPos = busModel * glm::vec4(pointLights[i].position, 1.0f);

      std::string num = std::to_string(i);
      shader.setVec3("pointLights[" + num + "].position", glm::vec3(worldPos));
      shader.setVec3("pointLights[" + num + "].ambient",
                     pointLights[i].ambient);
      shader.setVec3("pointLights[" + num + "].diffuse",
                     pointLights[i].diffuse);
      shader.setVec3("pointLights[" + num + "].specular",
                     pointLights[i].specular);
      shader.setFloat("pointLights[" + num + "].constant",
                      pointLights[i].constant);
      shader.setFloat("pointLights[" + num + "].linear", pointLights[i].linear);
      shader.setFloat("pointLights[" + num + "].quadratic",
                      pointLights[i].quadratic);
      shader.setBool("pointLights[" + num + "].enabled",
                     true); // Controlled by global toggle usually
    }

    // Pass Spot Light (Headlight - Attached to Bus)
    // Offset headlight relative to bus
    glm::vec3 headlightLocalPos(0.0f, -0.2f, 2.5f);
    glm::vec3 headlightLocalDir(0.0f, -0.2f, 1.0f);
    glm::vec3 hlPos = glm::vec3(busModel * glm::vec4(headlightLocalPos, 1.0f));
    glm::vec3 hlDir = glm::normalize(glm::mat3(busModel) * headlightLocalDir);

    shader.setVec3("spotLight.position", hlPos);
    shader.setVec3("spotLight.direction", hlDir);
    shader.setVec3("spotLight.ambient", spotLight.ambient);
    shader.setVec3("spotLight.diffuse", spotLight.diffuse);
    shader.setVec3("spotLight.specular", spotLight.specular);
    shader.setFloat("spotLight.constant", spotLight.constant);
    shader.setFloat("spotLight.linear", spotLight.linear);
    shader.setFloat("spotLight.quadratic", spotLight.quadratic);
    shader.setFloat("spotLight.cutOff", spotLight.cutOff);
    shader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);

    // RENDER 4 VIEWPORTS
    for (int i = 0; i < 4; i++) {
      // Set Viewport Area
      glViewport(viewports[i].x, viewports[i].y, viewports[i].width,
                 viewports[i].height);
      glScissor(viewports[i].x, viewports[i].y, viewports[i].width,
                viewports[i].height);
      glClear(
          GL_DEPTH_BUFFER_BIT); // Clear depth for this viewport (keep color)

      // Set Camera
      glm::mat4 projection = glm::perspective(
          glm::radians(45.0f),
          (float)viewports[i].width / (float)viewports[i].height, 0.1f, 100.0f);
      glm::mat4 view = GetViewportViewMatrix(i, busPos, busYaw);
      shader.setMat4("projection", projection);
      shader.setMat4("view", view);
      shader.setVec3("viewPos", glm::vec3(glm::inverse(view)[3]));

      // Set Lighting Config
      SetupViewportLighting(shader, i);

      // Draw Scene
      // 1. Road
      glm::mat4 roadM = glm::translate(glm::mat4(1.0f), glm::vec3(0, -1.0, 0));
      roadM = glm::scale(roadM, glm::vec3(100.0f, 0.1f, 100.0f));
      shader.setVec3("material.ambient", 0.2f, 0.2f, 0.2f);
      shader.setVec3("material.diffuse", 0.2f, 0.2f, 0.2f); // Dark Grey
      shader.setVec3("material.specular", 0.1f, 0.1f, 0.1f);
      shader.setFloat("material.shininess", 16.0f);
      roadCube.draw(shader.ID, roadM);

      // 2. Bus
      bus.Draw(shader, busModel);

      // 3. Emissive Light Spheres (Visualizing the point lights)
      if (pointLightOn && i == 0) { // Only show bulbs in TL (optional) or all
        // Show Bulb visual if light is ON
        for (int j = 0; j < pointLights.size(); j++) {
          glm::vec4 wPos = busModel * glm::vec4(pointLights[j].position, 1.0f);
          glm::mat4 bulbM = glm::translate(glm::mat4(1.0f), glm::vec3(wPos));
          bulbM = glm::scale(bulbM, glm::vec3(0.05f));

          shader.setVec3("material.emissive", 1.0f, 1.0f, 0.8f);
          roadCube.draw(shader.ID, bulbM); // Reusing cube for bulb
          shader.setVec3("material.emissive", 0.0f, 0.0f, 0.0f);
        }
      }
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
