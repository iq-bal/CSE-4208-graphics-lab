
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>
#include <vector>

#include "audio.h"
#include "camera.h"
#include "geometry.h"
#include "shader.h"

// STB Image implementation
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// Camera
Camera camera(glm::vec3(0.0f, 1.5f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Interaction State
bool bladeActive = true;
float bladeAngle = 0.0f;
float bladeTime = 0.0f;

bool sarcophagusOpen = false;
float sarcophagusSlide = 0.0f;
bool sarcophagusInteract = false;

// Lighting
glm::vec3 lightPos(0.0f, 2.0f, 0.0f); // Central light

// Function prototypes
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// Hierarchical Drawing Functions
void drawBladeTrap(Shader &shader, Cube &cube, Cylinder &cyl,
                   glm::mat4 parentModel, float time);
void drawSarcophagus(Shader &shader, Cube &cube, glm::mat4 parentModel,
                     float slideAmount);
void drawPillar(Shader &shader, Cube &cube, glm::mat4 model);
void drawTorch(Shader &shader, Cube &cube, glm::mat4 model);

int main() {
  // glfw: initialize and configure
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "The Crypt of Thoth", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  // tell GLFW to capture our mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // glew init
  if (glewInit() != GLEW_OK) {
    std::cout << "Failed to initialize GLEW" << std::endl;
    return -1;
  }

  // configure global opengl state
  glEnable(GL_DEPTH_TEST);

  // Start background music
  startBackgroundMusic("resources/arabian_nights.mp3");

  // build and compile shaders
  Shader mainShader("vshader.glsl", "fshader.glsl");

  // Geometry
  Cube cube;
  Cylinder cylinder(36);

  // Textures (Mock or Real)
  // unsigned int diffuseMap = loadTexture("container2.png");
  // unsigned int specularMap = loadTexture("container2_specular.png");

  // Shader config
  mainShader.use();
  mainShader.setInt("texture1", 0);
  mainShader.setInt("normalMap", 1);

  // Render loop
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    // Logic
    bladeTime += deltaTime;
    if (sarcophagusInteract) {
      if (sarcophagusOpen && sarcophagusSlide < 2.5f)
        sarcophagusSlide += deltaTime;
      if (!sarcophagusOpen && sarcophagusSlide > 0.0f)
        sarcophagusSlide -= deltaTime;
    }

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mainShader.use();

    // View/Proj
    glm::mat4 projection =
        glm::perspective(glm::radians(camera.Zoom),
                         (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    mainShader.setMat4("projection", projection);
    mainShader.setMat4("view", view);
    mainShader.setVec3("viewPos", camera.Position);

    // Lighting
    // Torch 1
    mainShader.setVec3("pointLights[0].position", 0.0f, 2.0f, -2.0f);
    mainShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    mainShader.setVec3("pointLights[0].diffuse", 1.0f, 0.6f,
                       0.2f); // Fire Color
    mainShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    mainShader.setFloat("pointLights[0].constant", 1.0f);
    mainShader.setFloat("pointLights[0].linear", 0.09f);
    mainShader.setFloat("pointLights[0].quadratic", 0.032f);

    // Sarcophagus Glow (Light 2)
    mainShader.setVec3("pointLights[1].position", 0.0f, 1.0f, -15.0f);
    mainShader.setVec3("pointLights[1].ambient", 0.0f, 0.0f, 0.0f);
    mainShader.setVec3("pointLights[1].diffuse", 0.0f, 0.2f,
                       0.8f); // Blue Mystical
    mainShader.setVec3("pointLights[1].specular", 0.5f, 0.5f, 0.5f);
    mainShader.setFloat("pointLights[1].constant", 1.0f);
    mainShader.setFloat("pointLights[1].linear", 0.22f);
    mainShader.setFloat("pointLights[1].quadratic", 0.20f);

    mainShader.setInt("numPointLights", 2);

    // SpotLight (Flashlight)
    mainShader.setVec3("spotLight.position", camera.Position);
    mainShader.setVec3("spotLight.direction", camera.Front);
    mainShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    mainShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    mainShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    mainShader.setFloat("spotLight.constant", 1.0f);
    mainShader.setFloat("spotLight.linear", 0.09f);
    mainShader.setFloat("spotLight.quadratic", 0.032f);
    mainShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    mainShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
    mainShader.setBool("spotLightOn", true); // Always on for exploration

    // --- DRAW SCENE ---

    // 1. Floor
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -1.0f, -5.0f));
    model = glm::scale(model, glm::vec3(10.0f, 0.1f, 30.0f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.4f, 0.4f, 0.4f); // Grey Stone
    mainShader.setBool("useTexture", false);             // For now
    cube.draw(mainShader.ID);

    // 2. Ceiling
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 4.0f, -5.0f));
    model = glm::scale(model, glm::vec3(10.0f, 0.1f, 30.0f));
    mainShader.setMat4("model", model);
    cube.draw(mainShader.ID);

    // 3. Walls (Simple Corridor)
    // Left
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 1.5f, -5.0f));
    model = glm::scale(model, glm::vec3(0.2f, 5.0f, 30.0f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.6f, 0.5f, 0.3f); // Sandstone
    cube.draw(mainShader.ID);
    // Right
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(5.0f, 1.5f, -5.0f));
    model = glm::scale(model, glm::vec3(0.2f, 5.0f, 30.0f));
    mainShader.setMat4("model", model);
    cube.draw(mainShader.ID);

    // 4. Pillars
    for (int i = 0; i < 5; i++) {
      glm::mat4 pm = glm::mat4(1.0f);
      pm = glm::translate(pm, glm::vec3(-4.0f, 0.0f, -i * 5.0f));
      drawPillar(mainShader, cube, pm);

      pm = glm::mat4(1.0f);
      pm = glm::translate(pm, glm::vec3(4.0f, 0.0f, -i * 5.0f));
      drawPillar(mainShader, cube, pm);
    }

    // 5. Blade Trap (Hierarchical)
    glm::mat4 trapPos = glm::mat4(1.0f);
    trapPos =
        glm::translate(trapPos, glm::vec3(0.0f, 3.5f, -8.0f)); // Ceiling mount
    drawBladeTrap(mainShader, cube, cylinder, trapPos, bladeTime);

    // 6. Sarcophagus (Hierarchical + Interactive)
    glm::mat4 sarcPos = glm::mat4(1.0f);
    sarcPos = glm::translate(sarcPos, glm::vec3(0.0f, -0.5f, -15.0f));
    drawSarcophagus(mainShader, cube, sarcPos, sarcophagusSlide);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  stopBackgroundMusic();
  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, deltaTime);

  // Interaction
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    float dist = glm::length(camera.Position - glm::vec3(0.0f, 0.0f, -15.0f));
    if (dist < 5.0f) {
      sarcophagusInteract = true;
      sarcophagusOpen = !sarcophagusOpen; // Toggle
    }
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset =
      lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera.ProcessMouseScroll(yoffset);
}

void drawBladeTrap(Shader &shader, Cube &cube, Cylinder &cyl,
                   glm::mat4 parentModel, float time) {
  // 1. Ceiling Mount (Static relative to parent)
  glm::mat4 mount = glm::scale(parentModel, glm::vec3(0.5f, 0.5f, 0.5f));
  shader.setMat4("model", mount);
  shader.setVec3("objectColor", 0.3f, 0.3f, 0.3f);
  cube.draw(shader.ID);

  // 2. Pendulum Arm (Rotates)
  float angle = glm::sin(time * 2.0f) * 1.0f; // Swing +/- 1 radian

  glm::mat4 armModel = glm::rotate(
      parentModel, angle,
      glm::vec3(0.0f, 0.0f,
                1.0f)); // Rotate around Z (Swing in XY plane? No, swing across
                        // corridor means rotate around Z axis)
  // Actually, if corridor is along Z, we want to swing ACROSS it (Left-Right).
  // That means rotating around Z axis.

  // Arm visual
  glm::mat4 armVis =
      glm::translate(armModel, glm::vec3(0.0f, -1.5f, 0.0f)); // Move down
  armVis = glm::scale(armVis, glm::vec3(0.1f, 3.0f, 0.1f));
  shader.setMat4("model", armVis);
  shader.setVec3("objectColor", 0.5f, 0.4f, 0.3f); // Rope/Chain
  cube.draw(shader.ID);

  // 3. Blade (Child of Arm)
  glm::mat4 bladeModel = glm::translate(armModel, glm::vec3(0.0f, -3.0f, 0.0f));
  bladeModel = glm::scale(bladeModel, glm::vec3(1.5f, 0.5f, 0.1f));
  shader.setMat4("model", bladeModel);
  shader.setVec3("objectColor", 0.8f, 0.8f, 0.9f); // Steel
  cube.draw(shader.ID);
}

void drawPillar(Shader &shader, Cube &cube, glm::mat4 model) {
  // Base
  glm::mat4 base = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
  base = glm::scale(base, glm::vec3(1.0f, 1.0f, 1.0f));
  shader.setMat4("model", base);
  shader.setVec3("objectColor", 0.6f, 0.6f, 0.5f);
  cube.draw(shader.ID);

  // Shaft
  glm::mat4 shaft = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0f));
  shaft = glm::scale(shaft, glm::vec3(0.8f, 3.0f, 0.8f));
  shader.setMat4("model", shaft);
  cube.draw(shader.ID);

  // Capital
  glm::mat4 cap = glm::translate(model, glm::vec3(0.0f, 3.2f, 0.0f));
  cap = glm::scale(cap, glm::vec3(1.2f, 0.4f, 1.2f));
  shader.setMat4("model", cap);
  cube.draw(shader.ID);
}

void drawSarcophagus(Shader &shader, Cube &cube, glm::mat4 parentModel,
                     float slideAmount) {
  // Base
  glm::mat4 base = glm::scale(parentModel, glm::vec3(1.5f, 1.0f, 3.0f));
  shader.setMat4("model", base);
  shader.setVec3("objectColor", 0.8f, 0.7f, 0.2f); // Gold/Stone
  cube.draw(shader.ID);

  // Lid (Sliding)
  glm::mat4 lid = glm::translate(
      parentModel, glm::vec3(0.0f, 0.6f, slideAmount)); // Slide along Z
  lid = glm::scale(lid, glm::vec3(1.6f, 0.2f, 3.1f));
  shader.setMat4("model", lid);
  shader.setVec3("objectColor", 0.9f, 0.8f, 0.3f); // Brighter Gold
  cube.draw(shader.ID);
}

unsigned int loadTexture(const char *path) {
  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}
