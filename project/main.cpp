
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
void drawLantern(Shader &shader, Cube &cube, Cylinder &cyl, glm::mat4 model);

// Lantern positions: 4 per side, alternating along Z
struct LanternInfo {
  glm::vec3 position;
  float facingX; // +1.0 for right-facing (on left wall), -1.0 for left-facing
                 // (on right wall)
};

const LanternInfo lanterns[] = {
    // Staggered arrangement (4 per side)
    {{-4.75f, 2.2f, -2.5f}, 1.0f},  {{-4.75f, 2.2f, -12.5f}, 1.0f},
    {{-4.75f, 2.2f, -22.5f}, 1.0f}, {{-4.75f, 2.2f, -32.5f}, 1.0f},

    {{4.75f, 2.2f, -7.5f}, -1.0f},  {{4.75f, 2.2f, -17.5f}, -1.0f},
    {{4.75f, 2.2f, -27.5f}, -1.0f}, {{4.75f, 2.2f, -37.5f}, -1.0f},
};
const int NUM_LANTERNS = 8;

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

  // Load textures
  unsigned int wallTexture = loadTexture("resources/wall_texture.png");
  unsigned int floorTexture = loadTexture("resources/floor_texture.png");

  // Shader config
  mainShader.use();
  mainShader.setInt("texture1", 0);
  mainShader.setInt("normalMap", 1);
  mainShader.setBool("useEmissive", false);
  mainShader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));

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

    // ========== LIGHTING ==========
    // 8 lantern point lights with warm fire color
    for (int i = 0; i < NUM_LANTERNS; i++) {
      std::string prefix = "pointLights[" + std::to_string(i) + "]";
      // Slight flicker effect
      float flicker = 0.9f + 0.1f * sin(currentFrame * 8.0f + i * 1.7f);
      mainShader.setVec3(prefix + ".position",
                         lanterns[i].position +
                             glm::vec3(lanterns[i].facingX * 0.3f, 0.3f, 0.0f));
      mainShader.setVec3(prefix + ".ambient", 0.06f, 0.04f, 0.02f);
      mainShader.setVec3(prefix + ".diffuse", 1.0f * flicker, 0.55f * flicker,
                         0.15f * flicker);
      mainShader.setVec3(prefix + ".specular", 0.6f, 0.4f, 0.1f);
      mainShader.setFloat(prefix + ".constant", 1.0f);
      mainShader.setFloat(prefix + ".linear", 0.22f); // Sharper falloff
      mainShader.setFloat(prefix + ".quadratic", 0.12f);
    }
    mainShader.setInt("numPointLights", NUM_LANTERNS);

    // SpotLight (Flashlight) – dim for atmosphere
    mainShader.setVec3("spotLight.position", camera.Position);
    mainShader.setVec3("spotLight.direction", camera.Front);
    mainShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    mainShader.setVec3("spotLight.diffuse", 0.4f, 0.35f, 0.25f); // Dim warm
    mainShader.setVec3("spotLight.specular", 0.3f, 0.3f, 0.3f);
    mainShader.setFloat("spotLight.constant", 1.0f);
    mainShader.setFloat("spotLight.linear", 0.14f);
    mainShader.setFloat("spotLight.quadratic", 0.07f);
    mainShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(14.0f)));
    mainShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(18.0f)));
    mainShader.setBool("spotLightOn", true);

    // ========== DRAW SCENE ==========
    mainShader.setBool("useEmissive", false);
    mainShader.setBool("useNormalMap", false);
    mainShader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));

    // Draw Floor (Continuous)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -1.0f, -15.0f));
    model = glm::scale(model, glm::vec3(10.0f, 0.1f, 50.0f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.6f, 0.55f, 0.5f);
    mainShader.setBool("useTexture", true);
    mainShader.setVec2("uvScale", glm::vec2(5.0f, 25.0f));
    cube.draw(mainShader.ID);

    // Segmented Walls, Ceiling, and Dividers
    for (int i = 0; i < 10; i++) {
      float zPos = -i * 5.0f;

      // --- 1. Vertical Dividers (Wall Columns) ---
      mainShader.setBool("useTexture", false);
      mainShader.setVec3("objectColor", 0.65f, 0.55f, 0.4f);
      // Left Divider
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(-4.85f, 1.5f, zPos));
      model = glm::scale(model, glm::vec3(0.35f, 5.0f, 0.5f));
      mainShader.setMat4("model", model);
      cube.draw(mainShader.ID);
      // Right Divider
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(4.85f, 1.5f, zPos));
      model = glm::scale(model, glm::vec3(0.35f, 5.0f, 0.5f));
      mainShader.setMat4("model", model);
      cube.draw(mainShader.ID);

      // --- 2. Ceiling Beams ---
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(0.0f, 3.85f, zPos));
      model = glm::scale(model, glm::vec3(10.0f, 0.35f, 0.5f));
      mainShader.setMat4("model", model);
      cube.draw(mainShader.ID);

      // --- 3. Wall Panels (between dividers) ---
      mainShader.setBool("useTexture", true);
      glBindTexture(GL_TEXTURE_2D, wallTexture);
      mainShader.setVec3("objectColor", 0.7f, 0.6f, 0.4f);
      mainShader.setVec2("uvScale", glm::vec2(0.8f, 1.0f)); // Large figures

      // Left Panel
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(-5.0f, 1.5f, zPos - 2.5f));
      model = glm::scale(model, glm::vec3(0.2f, 5.0f, 4.5f));
      mainShader.setMat4("model", model);
      cube.draw(mainShader.ID);
      // Right Panel
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(5.0f, 1.5f, zPos - 2.5f));
      model = glm::scale(model, glm::vec3(0.2f, 5.0f, 4.5f));
      mainShader.setMat4("model", model);
      cube.draw(mainShader.ID);

      // --- 4. Ceiling Panels ---
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(0.0f, 4.05f, zPos - 2.5f));
      model = glm::scale(model, glm::vec3(10.0f, 0.1f, 4.5f));
      mainShader.setMat4("model", model);
      mainShader.setVec3("objectColor", 0.45f, 0.35f, 0.25f);
      cube.draw(mainShader.ID);
    }

    // Back wall
    mainShader.setBool("useTexture", true);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 1.5f, -50.0f));
    model = glm::scale(model, glm::vec3(10.0f, 5.0f, 0.2f));
    mainShader.setMat4("model", model);
    mainShader.setVec2("uvScale", glm::vec2(2.0f, 1.0f));
    cube.draw(mainShader.ID);

    mainShader.setBool("useTexture", false);
    mainShader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));

    // 4. Pillars removed (as requested)

    // 5. Wall-mounted Lanterns
    for (int i = 0; i < NUM_LANTERNS; i++) {
      glm::mat4 lm = glm::mat4(1.0f);
      lm = glm::translate(lm, lanterns[i].position);
      // Scale facing direction
      lm = glm::scale(lm, glm::vec3(lanterns[i].facingX, 1.0f, 1.0f));
      drawLantern(mainShader, cube, cylinder, lm);
    }

    // 6. Blade Trap (Hierarchical)
    glm::mat4 trapPos = glm::mat4(1.0f);
    trapPos =
        glm::translate(trapPos, glm::vec3(0.0f, 3.5f, -8.0f)); // Ceiling mount
    drawBladeTrap(mainShader, cube, cylinder, trapPos, bladeTime);

    // 7. Sarcophagus (Hierarchical + Interactive)
    glm::mat4 sarcPos = glm::mat4(1.0f);
    sarcPos = glm::translate(sarcPos, glm::vec3(0.0f, -0.5f, -20.0f));
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
    float dist = glm::length(camera.Position - glm::vec3(0.0f, 0.0f, -20.0f));
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

void drawLantern(Shader &shader, Cube &cube, Cylinder &cyl, glm::mat4 model) {
  // Wall bracket (dark iron)
  shader.setBool("useEmissive", false);
  glm::mat4 bracket = glm::translate(model, glm::vec3(0.15f, 0.0f, 0.0f));
  bracket = glm::scale(bracket, glm::vec3(0.3f, 0.08f, 0.08f));
  shader.setMat4("model", bracket);
  shader.setVec3("objectColor", 0.15f, 0.12f, 0.08f); // Dark iron
  cube.draw(shader.ID);

  // Vertical stick / torch handle
  glm::mat4 stick = glm::translate(model, glm::vec3(0.3f, 0.0f, 0.0f));
  stick = glm::scale(stick, glm::vec3(0.05f, 0.6f, 0.05f));
  shader.setMat4("model", stick);
  shader.setVec3("objectColor", 0.25f, 0.18f, 0.1f); // Wood brown
  cube.draw(shader.ID);

  // Torch cup / holder at top
  glm::mat4 cup = glm::translate(model, glm::vec3(0.3f, 0.3f, 0.0f));
  cup = glm::scale(cup, glm::vec3(0.15f, 0.1f, 0.15f));
  shader.setMat4("model", cup);
  shader.setVec3("objectColor", 0.2f, 0.15f, 0.08f); // Dark metal
  cube.draw(shader.ID);

  // Flame (emissive – glows bright orange-yellow)
  shader.setBool("useEmissive", true);
  shader.setVec3("emissiveColor", 1.0f, 0.7f, 0.2f); // Bright fire
  glm::mat4 flame = glm::translate(model, glm::vec3(0.3f, 0.45f, 0.0f));
  flame = glm::scale(flame, glm::vec3(0.1f, 0.18f, 0.1f));
  shader.setMat4("model", flame);
  cube.draw(shader.ID);

  // Outer flame glow (larger, dimmer)
  shader.setVec3("emissiveColor", 0.9f, 0.45f, 0.05f); // Deeper orange
  glm::mat4 glow = glm::translate(model, glm::vec3(0.3f, 0.5f, 0.0f));
  glow = glm::scale(glow, glm::vec3(0.15f, 0.12f, 0.15f));
  shader.setMat4("model", glow);
  cube.draw(shader.ID);

  shader.setBool("useEmissive", false);
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
