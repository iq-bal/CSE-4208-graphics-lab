#include "cube.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Camera settings (Global/Main Camera)
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float roll = 0.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Bus state
glm::vec3 busPos = glm::vec3(0.0f, 0.0f, 0.0f);
float busYaw = 0.0f;
float busSpeed = 5.0f;
float fanAngle = 0.0f;
bool fanRotating = false;
float doorOpen = 0.0f;
bool doorOpening = false;
float windowOpen = 0.0f;
bool windowOpening = false;

// Lighting Toggles (Global State)
bool dirLightOn = true;
bool pointLightOn = true;
bool spotLightOn = true;
bool ambientOn = true;
bool diffuseOn = true;
bool specularOn = true;
bool emissiveOn = true;

// Viewport Camera Modes
enum CameraMode { CAM_ISO, CAM_TOP, CAM_FRONT, CAM_INSIDE };

struct ViewportState {
  CameraMode mode;
};

ViewportState viewports[4]; // 0: TL, 1: TR, 2: BL, 3: BR

// Custom lookAt function
glm::mat4 myLookAt(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp) {
  glm::vec3 zaxis = glm::normalize(position - target);
  glm::vec3 xaxis = glm::normalize(glm::cross(glm::normalize(worldUp), zaxis));
  glm::vec3 yaxis = glm::cross(zaxis, xaxis);

  glm::mat4 translation = glm::mat4(1.0f);
  translation[3][0] = -position.x;
  translation[3][1] = -position.y;
  translation[3][2] = -position.z;

  glm::mat4 rotation = glm::mat4(1.0f);
  rotation[0][0] = xaxis.x;
  rotation[1][0] = xaxis.y;
  rotation[2][0] = xaxis.z;
  rotation[0][1] = yaxis.x;
  rotation[1][1] = yaxis.y;
  rotation[2][1] = yaxis.z;
  rotation[0][2] = zaxis.x;
  rotation[1][2] = zaxis.y;
  rotation[2][2] = zaxis.z;

  return rotation * translation;
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  float cameraSpeed = 5.0f * deltaTime;
  // Standard WASD for Bus Movement (Primary)
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS &&
          glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
    busPos.x += sin(glm::radians(busYaw)) * busSpeed * deltaTime;
    busPos.z += cos(glm::radians(busYaw)) * busSpeed * deltaTime;
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    busPos.x -= sin(glm::radians(busYaw)) * busSpeed * deltaTime;
    busPos.z -= cos(glm::radians(busYaw)) * busSpeed * deltaTime;
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    busYaw += 50.0f * deltaTime;
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    busYaw -= 50.0f * deltaTime;

  // Viewport Camera Cycling (Q/W/E/R)
  // To avoid conflict with W (Move), require SHIFT for W cycling. Q, E, R are
  // free-ish (E is strafe in flycam usually, but here likely acceptable)
  // Actually, let's just use Q, Z, E, R? No, prompt said Q/W/E/R.
  // I will check if SHIFT is pressed for W cycle.

  static bool qPressed = false;
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    if (!qPressed) {
      viewports[0].mode = static_cast<CameraMode>((viewports[0].mode + 1) % 4);
      qPressed = true;
    }
  } else
    qPressed = false;

  static bool wPressed = false;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS &&
      glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
    if (!wPressed) {
      viewports[1].mode = static_cast<CameraMode>((viewports[1].mode + 1) % 4);
      wPressed = true;
    }
  } else
    wPressed = false;

  static bool ePressed = false;
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    if (!ePressed) {
      viewports[2].mode = static_cast<CameraMode>((viewports[2].mode + 1) % 4);
      ePressed = true;
    }
  } else
    ePressed = false;

  static bool rPressed = false;
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    if (!rPressed) {
      viewports[3].mode = static_cast<CameraMode>((viewports[3].mode + 1) % 4);
      rPressed = true;
    }
  } else
    rPressed = false;

  // Lighting Toggles
  static bool key1Pressed = false;
  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
    if (!key1Pressed) {
      dirLightOn = !dirLightOn;
      key1Pressed = true;
    }
  } else
    key1Pressed = false;
  static bool key2Pressed = false;
  if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
    if (!key2Pressed) {
      pointLightOn = !pointLightOn;
      key2Pressed = true;
    }
  } else
    key2Pressed = false;
  static bool key3Pressed = false;
  if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
    if (!key3Pressed) {
      spotLightOn = !spotLightOn;
      key3Pressed = true;
    }
  } else
    key3Pressed = false;
  static bool key4Pressed = false;
  if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
    if (!key4Pressed) {
      emissiveOn = !emissiveOn;
      key4Pressed = true;
    }
  } else
    key4Pressed = false;
  static bool key5Pressed = false;
  if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
    if (!key5Pressed) {
      ambientOn = !ambientOn;
      key5Pressed = true;
    }
  } else
    key5Pressed = false;
  static bool key6Pressed = false;
  if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
    if (!key6Pressed) {
      diffuseOn = !diffuseOn;
      key6Pressed = true;
    }
  } else
    key6Pressed = false;
  static bool key7Pressed = false;
  if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
    if (!key7Pressed) {
      specularOn = !specularOn;
      key7Pressed = true;
    }
  } else
    key7Pressed = false;

  // Bus controls
  static bool gPressed = false;
  if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
    if (!gPressed) {
      fanRotating = !fanRotating;
      gPressed = true;
    }
  } else
    gPressed = false;
  static bool oPressed = false;
  if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
    if (!oPressed) {
      doorOpening = !doorOpening;
      oPressed = true;
    }
  } else
    oPressed = false;
  static bool iPressed = false;
  if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
    if (!iPressed) {
      windowOpening = !windowOpening;
      iPressed = true;
    }
  } else
    iPressed = false;
}

std::string readShaderCode(const char *filePath) {
  std::ifstream shaderFile;
  shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    shaderFile.open(filePath);
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    return shaderStream.str();
  } catch (std::ifstream::failure &e) {
    std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << filePath
              << std::endl;
    return "";
  }
}

glm::mat4 GetViewMatrix(CameraMode mode) {
  switch (mode) {
  case CAM_ISO: {
    glm::vec3 pos = busPos + glm::vec3(-10.0f, 10.0f, 10.0f);
    return myLookAt(pos, busPos, glm::vec3(0.0f, 1.0f, 0.0f));
  }
  case CAM_TOP: {
    glm::vec3 pos =
        busPos +
        glm::vec3(0.0f, 15.0f,
                  0.1f); // Offset Z slightly to avoid up-vector singularity
    return myLookAt(
        pos, busPos,
        glm::vec3(0.0f, 1.0f,
                  0.0f)); // Z is up? No, Y is up, but looking down Y requires
                          // specific up vector logic usually (like -Z), but
                          // myLookAt handles it if cross product works.
    // If Looking down -Y (0, -1, 0), and Up is (0, 0, -1)...
    // pos - target = (0, 15, 0.1). norm = (0, 1, 0) approx.
    // Up = (0, 1, 0).
    // Cross(Up, Z) => 0 vector! Singularity.
    // Let's use Up = (0, 0, -1) for Top View.
    return myLookAt(pos, busPos, glm::vec3(0.0f, 0.0f, -1.0f));
  }
  case CAM_FRONT: {
    // Front of the BUS, so look at busPos from +Z
    glm::vec3 pos = busPos + glm::vec3(0.0f, 2.0f, 12.0f);
    return myLookAt(pos, busPos, glm::vec3(0.0f, 1.0f, 0.0f));
  }
  case CAM_INSIDE: {
    glm::vec3 pos = busPos + glm::vec3(0.0f, 0.5f, 2.0f); // Driver seat pos
    // Look forward
    glm::vec3 target;
    target.x = pos.x + sin(glm::radians(busYaw));
    target.z = pos.z + cos(glm::radians(busYaw));
    target.y = pos.y;
    return myLookAt(pos, target, glm::vec3(0.0f, 1.0f, 0.0f));
  }
  }
  return glm::mat4(1.0f);
}

void renderScene(unsigned int shaderProgram, Cube &busBody, Sphere &wheel,
                 Cube &windowPane, Cube &door, Cube &fan, Cube &windshield) {
  // --- STATIC ENVIRONMENT (ROAD) ---
  // Drawn first, unaffected by bus position
  glm::mat4 roadModel = glm::mat4(1.0f);
  roadModel = glm::translate(roadModel, glm::vec3(0.0f, -1.0f, 0.0f));
  roadModel = glm::scale(roadModel, glm::vec3(200.0f, 0.1f, 200.0f));
  glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1,
               &glm::vec3(0.2f, 0.2f, 0.2f)[0]); // Dark asphalt
  busBody.draw(shaderProgram, roadModel);
  // --- END ROAD ---

  // Model Matrix
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, busPos);
  model = glm::rotate(model, glm::radians(busYaw), glm::vec3(0, 1, 0));

  // Colors
  glm::vec3 bodyColor(0.8f, 0.8f, 0.8f);
  glm::vec3 windowColor(0.0f, 0.5f, 0.8f);
  glm::vec3 tireColor(0.1f, 0.1f, 0.1f);
  glm::vec3 doorColor(0.5f, 0.3f, 0.1f);
  glm::vec3 fanColor(1.0f, 1.0f, 0.0f);
  glm::vec3 seatColor(0.2f, 0.2f, 0.8f);

  // --- INTERIOR LIGHT BULBS ---
  // Drawn relative to bus so they move with it
  glm::vec3 lightOffsets[] = {
      glm::vec3(0.0f, 0.6f, 1.5f),  // Front Ceiling (Inside)
      glm::vec3(0.0f, 0.6f, 0.0f),  // Middle Ceiling (Inside)
      glm::vec3(0.0f, 0.6f, -1.5f), // Back Ceiling (Inside)
      glm::vec3(0.0f, 0.6f, 0.75f)  // Extra Mid-Front (Inside)
  };

  // Temporarily enable emissive for bulbs so they glow
  glUniform1i(glGetUniformLocation(shaderProgram, "emissiveOn"), true);
  glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1,
               &glm::vec3(1.0f, 0.9f, 0.6f)[0]);

  for (int i = 0; i < 4; i++) {
    // 1. Fixture (Stem) - Non-emissive
    glUniform1i(glGetUniformLocation(shaderProgram, "emissiveOn"), false);
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1,
                 &glm::vec3(0.2f, 0.2f, 0.2f)[0]); // Dark Metal
    glm::mat4 fixtureM = glm::translate(
        model, glm::vec3(lightOffsets[i].x, 0.65f,
                         lightOffsets[i].z)); // Start slightly above bulb
    fixtureM = glm::scale(fixtureM, glm::vec3(0.02f, 0.1f, 0.02f)); // Thin stem
    busBody.draw(shaderProgram, fixtureM);

    // 2. Bulb (Sphere) - Standard Emission
    glUniform1i(glGetUniformLocation(shaderProgram, "emissiveOn"), true);
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1,
                 &glm::vec3(1.0f, 0.9f, 0.7f)[0]);

    glm::mat4 bulbM = glm::translate(model, lightOffsets[i]);
    bulbM = glm::scale(bulbM, glm::vec3(0.08f, 0.08f, 0.08f)); // Small sphere
    wheel.draw(shaderProgram, bulbM);
  }
  // Turn back off if it wasn't supposed to be on generally (safest bet),
  // but better: just let the next objectColor set define it.
  // Emissive logic in main loop resets 'locEmit' before renderScene, but inside
  // renderScene we persist state. So we MUST disable it after drawing bulbs to
  // avoid making the floor glowing next!
  glUniform1i(glGetUniformLocation(shaderProgram, "emissiveOn"), false);

  // --- HOLLOW BUS BODY CONSTRUCTION ---

  // 1. Floor
  glm::mat4 floorM = glm::translate(model, glm::vec3(0.0f, -0.7f, 0.0f));
  floorM = glm::scale(floorM, glm::vec3(2.0f, 0.1f, 5.0f));
  glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1,
               &bodyColor[0]);
  busBody.draw(shaderProgram, floorM);

  // 2. Roof
  glm::mat4 roofM = glm::translate(model, glm::vec3(0.0f, 0.7f, 0.0f));
  roofM = glm::scale(roofM, glm::vec3(2.0f, 0.1f, 5.0f));
  busBody.draw(shaderProgram, roofM);

  // 3. Lower Side Walls (below windows)
  // Left Wall Lower
  glm::mat4 lwM = glm::translate(model, glm::vec3(-0.95f, -0.35f, 0.0f));
  lwM = glm::scale(lwM, glm::vec3(0.1f, 0.6f, 5.0f));
  busBody.draw(shaderProgram, lwM);
  // Right Wall Lower (with gap for door)
  // Front part
  glm::mat4 rwM1 = glm::translate(model, glm::vec3(0.95f, -0.35f, -1.0f));
  rwM1 = glm::scale(rwM1, glm::vec3(0.1f, 0.6f, 3.0f));
  busBody.draw(shaderProgram, rwM1);
  // Back part
  glm::mat4 rwM2 = glm::translate(model, glm::vec3(0.95f, -0.35f, 2.0f));
  rwM2 = glm::scale(rwM2, glm::vec3(0.1f, 0.6f, 1.0f));
  busBody.draw(shaderProgram, rwM2);

  // 4. Upper Structure (Pillars between windows) - Simplified as thin vertical
  // strips
  for (int i = 0; i < 4; i++) {
    float z = -2.0f + i * 1.5f;
    glm::mat4 pM = glm::translate(model, glm::vec3(-0.95f, 0.2f, z));
    pM = glm::scale(pM, glm::vec3(0.1f, 0.9f, 0.1f));
    busBody.draw(shaderProgram, pM);
    pM = glm::translate(model, glm::vec3(0.95f, 0.2f, z));
    pM = glm::scale(pM, glm::vec3(0.1f, 0.9f, 0.1f));
    busBody.draw(shaderProgram, pM);
  }

  // 5. Front/Back Walls
  // Front
  glm::mat4 fwM = glm::translate(model, glm::vec3(0.0f, 0.0f, 2.45f));
  fwM = glm::scale(fwM, glm::vec3(2.0f, 1.5f, 0.1f));
  busBody.draw(shaderProgram, fwM);
  // Back
  glm::mat4 bwM = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.45f));
  bwM = glm::scale(bwM, glm::vec3(2.0f, 1.5f, 0.1f));
  busBody.draw(shaderProgram, bwM);

  // --- INTERIOR SEATS ---
  glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1,
               &seatColor[0]);
  for (int i = 0; i < 4; i++) {
    // Left Row
    glm::mat4 seatL =
        glm::translate(model, glm::vec3(-0.6f, -0.4f, -1.5f + i * 0.8f));
    seatL = glm::scale(seatL, glm::vec3(0.5f, 0.1f, 0.5f)); // Simple seat squab
    busBody.draw(shaderProgram, seatL);
    glm::mat4 backL =
        glm::translate(model, glm::vec3(-0.6f, -0.1f, -1.7f + i * 0.8f));
    backL = glm::scale(backL, glm::vec3(0.5f, 0.5f, 0.1f)); // Seat back
    busBody.draw(shaderProgram, backL);

    // Right Row
    glm::mat4 seatR =
        glm::translate(model, glm::vec3(0.6f, -0.4f, -1.5f + i * 0.8f));
    seatR = glm::scale(seatR, glm::vec3(0.5f, 0.1f, 0.5f));
    busBody.draw(shaderProgram, seatR);
    glm::mat4 backR =
        glm::translate(model, glm::vec3(0.6f, -0.1f, -1.7f + i * 0.8f));
    backR = glm::scale(backR, glm::vec3(0.5f, 0.5f, 0.1f));
    busBody.draw(shaderProgram, backR);
  }

  // Windshield (Front Glass) - Adjusted position
  glm::mat4 wSM = glm::translate(model, glm::vec3(0.0f, 0.3f, 2.51f));
  wSM = glm::scale(wSM, glm::vec3(1.8f, 0.8f, 0.05f));
  glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1,
               &glm::vec3(0.0f, 0.7f, 0.9f)[0]);
  windshield.draw(shaderProgram, wSM);

  // Wheels
  glm::vec3 wheelSpecs[] = {
      glm::vec3(-1.1f, -0.75f, 2.0f), glm::vec3(1.1f, -0.75f, 2.0f),
      glm::vec3(-1.1f, -0.75f, -2.0f), glm::vec3(1.1f, -0.75f, -2.0f)};
  glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1,
               &tireColor[0]);
  for (int i = 0; i < 4; i++) {
    glm::mat4 wM = glm::translate(model, wheelSpecs[i]);
    wM = glm::rotate(wM, glm::radians(90.0f),
                     glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate to face outward
    wM = glm::scale(wM, glm::vec3(0.6f, 0.3f, 0.6f));
    wheel.draw(shaderProgram, wM);
  }

  // Door (Animating) - Adjusted
  if (doorOpening && doorOpen < 1.0f)
    doorOpen += deltaTime;
  if (!doorOpening && doorOpen > 0.0f)
    doorOpen -= deltaTime;
  glm::mat4 dM = glm::translate(model, glm::vec3(1.01f, -0.2f, 1.5f));
  dM =
      glm::translate(dM, glm::vec3(0.0f, 0.0f, doorOpen * -0.8f)); // Slide open
  dM = glm::scale(dM, glm::vec3(0.05f, 1.0f, 0.8f));
  glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1,
               &doorColor[0]);
  door.draw(shaderProgram, dM);

  // Windows (Animating) - Adjusted
  if (windowOpening && windowOpen < 1.0f)
    windowOpen += deltaTime;
  if (!windowOpening && windowOpen > 0.0f)
    windowOpen -= deltaTime;

  glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1,
               &windowColor[0]);
  for (int i = 0; i < 3; i++) {
    // Left windows
    glm::mat4 wML =
        glm::translate(model, glm::vec3(-1.01f, 0.3f, 1.5f - i * 1.5f));
    wML = glm::translate(wML, glm::vec3(0.0f, windowOpen * -0.4f, 0.0f));
    wML = glm::scale(wML, glm::vec3(0.05f, 0.6f, 1.0f));
    windowPane.draw(shaderProgram, wML);
    // Right windows
    if (i > 0) { // Skip door area
      glm::mat4 wMR =
          glm::translate(model, glm::vec3(1.01f, 0.3f, 1.5f - i * 1.5f));
      wMR = glm::translate(wMR, glm::vec3(0.0f, windowOpen * -0.4f, 0.0f));
      wMR = glm::scale(wMR, glm::vec3(0.05f, 0.6f, 1.0f));
      windowPane.draw(shaderProgram, wMR);
    }
  }
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window = glfwCreateWindow(
      1000, 800, "Assignment 03 - Precision Lighting", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glewInit();
  glEnable(GL_DEPTH_TEST);

  std::string shaderContent = readShaderCode("shaders.glsl");
  size_t splitPos = shaderContent.find("// <!-- split -->");
  std::string vertexCode, fragmentCode;
  if (splitPos != std::string::npos) {
    vertexCode = shaderContent.substr(0, splitPos);
    fragmentCode = shaderContent.substr(splitPos + 17);
  } else {
    std::cerr << "Could not split shader file!" << std::endl;
    return -1;
  }
  const char *vShaderCode = vertexCode.c_str();
  const char *fShaderCode = fragmentCode.c_str();

  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vShaderCode, NULL);
  glCompileShader(vertexShader);
  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }
  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }
  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  Cube busBody(glm::vec3(0.8f, 0.8f, 0.8f));
  Sphere wheel(glm::vec3(0.1f, 0.1f, 0.1f));
  Cube windowPane(glm::vec3(0.0f, 0.5f, 0.8f));
  Cube door(glm::vec3(0.5f, 0.3f, 0.1f));
  Cube fan(glm::vec3(1.0f, 1.0f, 0.0f));
  Cube windshield(glm::vec3(0.0f, 0.7f, 0.9f));

  // Initialize Viewports
  viewports[0].mode = CAM_ISO;    // TL: ISO (Combined Lighting)
  viewports[1].mode = CAM_TOP;    // TR: TOP (Ambient Only)
  viewports[2].mode = CAM_FRONT;  // BL: FRONT (Diffuse Only)
  viewports[3].mode = CAM_INSIDE; // BR: INSIDE (Directional Only)

  std::cout << "Controls:\n1-3: Toggle Lights\n4: Toggle Emissive\n5-7: Toggle "
               "Components\nArrows: Move Bus\nQ/Shift+W/E/R: Cycle Cameras per "
               "Viewport"
            << std::endl;

  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    processInput(window);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    // Common Lighting Setup (Positions/Colors)
    glUniform3fv(glGetUniformLocation(shaderProgram, "dirLight.direction"), 1,
                 &glm::vec3(-0.2f, -1.0f, -0.3f)[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "dirLight.ambient"), 1,
                 &glm::vec3(0.2f, 0.2f, 0.2f)[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "dirLight.diffuse"), 1,
                 &glm::vec3(0.4f, 0.4f, 0.4f)[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "dirLight.specular"), 1,
                 &glm::vec3(0.5f, 0.5f, 0.5f)[0]);

    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f), glm::vec3(0.0f, 0.0f, -3.0f)};
    for (int i = 0; i < 4; i++) {
      std::string number = std::to_string(i);
      glUniform3fv(
          glGetUniformLocation(
              shaderProgram, ("pointLights[" + number + "].position").c_str()),
          1, &pointLightPositions[i][0]);
      glUniform3fv(
          glGetUniformLocation(shaderProgram,
                               ("pointLights[" + number + "].ambient").c_str()),
          1, &glm::vec3(0.05f, 0.05f, 0.05f)[0]);

      // Standard High Intensity (Key 2 Toggles via uniform)
      glUniform3fv(
          glGetUniformLocation(shaderProgram,
                               ("pointLights[" + number + "].diffuse").c_str()),
          1, &glm::vec3(3.0f, 2.5f, 2.0f)[0]);
      glUniform3fv(
          glGetUniformLocation(
              shaderProgram, ("pointLights[" + number + "].specular").c_str()),
          1, &glm::vec3(1.0f, 1.0f, 1.0f)[0]);

      glUniform1f(
          glGetUniformLocation(
              shaderProgram, ("pointLights[" + number + "].constant").c_str()),
          1.0f);
      glUniform1f(
          glGetUniformLocation(shaderProgram,
                               ("pointLights[" + number + "].linear").c_str()),
          0.09f);
      glUniform1f(
          glGetUniformLocation(
              shaderProgram, ("pointLights[" + number + "].quadratic").c_str()),
          0.032f);
    }

    glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.position"), 1,
                 &cameraPos[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.direction"), 1,
                 &cameraFront[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.ambient"), 1,
                 &glm::vec3(0.0f, 0.0f, 0.0f)[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.diffuse"), 1,
                 &glm::vec3(1.0f, 1.0f, 1.0f)[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.specular"), 1,
                 &glm::vec3(1.0f, 1.0f, 1.0f)[0]);
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.constant"),
                1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.linear"), 0.09f);
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.quadratic"),
                0.032f);
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.cutOff"),
                glm::cos(glm::radians(12.5f)));

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    int halfW = width / 2;
    int halfH = height / 2;

    for (int i = 0; i < 4; i++) {
      // Viewport & Scissor Setup
      int x = 0, y = 0;
      if (i == 0) {
        x = 0;
        y = halfH;
      } // TL
      else if (i == 1) {
        x = halfW;
        y = halfH;
      } // TR
      else if (i == 2) {
        x = 0;
        y = 0;
      } // BL
      else if (i == 3) {
        x = halfW;
        y = 0;
      } // BR

      glViewport(x, y, halfW, halfH);

      // Lighting Configuration per Viewport (Strict Compliance)
      bool locDir = dirLightOn;
      bool locPoint = pointLightOn;
      bool locSpot = spotLightOn;
      bool locAmb = ambientOn;
      bool locDiff = diffuseOn;
      bool locSpec = specularOn;
      bool locEmit = emissiveOn;

      // TL (Combined): Use global state (already set)

      // TR (Ambient Only): Force Ambient ON, others OFF
      if (i == 1) {
        locDir = true;
        locPoint = false;
        locSpot = false;
        locAmb = true;
        locDiff = false;
        locSpec = false;
        locEmit = false;
      }
      // BL (Diffuse Only): Force Diffuse ON, others OFF
      if (i == 2) {
        locDir = true;
        locPoint = false;
        locSpot = false;
        locAmb = false;
        locDiff = true;
        locSpec = false;
        locEmit = false;
      }
      // BR (Inside View): Directional Lighting Only (Assignment Req)
      if (i == 3) {
        locDir = true;
        locPoint = false;
        locSpot = false;
        locAmb = false; // Assignment says "Directional Only", usually implies
                        // Ambient off?
        // Table says "Directional Only". Ambient is separate.
        // Wait, TR is "Ambient Only".
        // So BR should likely have NO Ambient if it's strictly Directional.
        // But without ambient, shadows are pitch black.
        // I will follow "Directional Only" literally: locAmb = false.
        locDiff =
            true; // Directional Light has Diffuse+Specular components usually.
        // "Directional Light" usually means the Sunlight source.
        // It contributes Ambient, Diffuse, Specular.
        // If "Directional Only" means "Only the Directional Light Source is
        // active", then we use locDir=true. AND we must ensure that the Global
        // Ambient (if any) is considered "Ambient Light". The assignment
        // separates "Ambient Light" (5) from "Directional Light" (1). So
        // "Directional Only" likely means: 1=ON, 2=OFF, 3=OFF. 5=OFF (Ambient
        // Term OFF), 6=ON (Diffuse Term ON), 7=ON (Specular Term ON). Let's
        // interpret "Directional Only" as "Only Directional Light logic is
        // enabled". The shader uses `dirLightOn` to toggle the *calculation* of
        // the directional light. If `dirLightOn` is true, it adds
        // Ambient+Diffuse+Specular from that light. If `locAmb` (Ambient Term
        // Toggle) is false, the shader might skip *all* ambient calculations?
        // Let's check shader logic later if needed. For now, strict
        // interpretation:
        locDir = true;
        locPoint = false;
        locSpot = false;
        locAmb = false;
        locDiff = true;
        locSpec = true;
        locEmit = false;
      }

      glUniform1i(glGetUniformLocation(shaderProgram, "dirLightOn"), locDir);
      glUniform1i(glGetUniformLocation(shaderProgram, "pointLightOn"),
                  locPoint);
      glUniform1i(glGetUniformLocation(shaderProgram, "spotLightOn"), locSpot);
      glUniform1i(glGetUniformLocation(shaderProgram, "ambientOn"), locAmb);
      glUniform1i(glGetUniformLocation(shaderProgram, "diffuseOn"), locDiff);
      glUniform1i(glGetUniformLocation(shaderProgram, "specularOn"), locSpec);
      glUniform1i(glGetUniformLocation(shaderProgram, "emissiveOn"), locEmit);

      // Camera View
      glm::mat4 view = GetViewMatrix(viewports[i].mode);
      glm::mat4 projection = glm::perspective(
          glm::radians(45.0f), (float)halfW / (float)halfH, 0.1f, 100.0f);

      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1,
                         GL_FALSE, &projection[0][0]);
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1,
                         GL_FALSE, &view[0][0]);

      // Extract view pos for specular calculation
      glm::vec3 viewPos = glm::vec3(glm::inverse(view)[3]);
      glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1,
                   &viewPos[0]);

      renderScene(shaderProgram, busBody, wheel, windowPane, door, fan,
                  windshield);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
