#include "cube.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Camera settings
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
bool lightOn = true;
float fanAngle = 0.0f;
bool fanRotating = false;
float doorOpen = 0.0f;
bool doorOpening = false;
float windowOpen = 0.0f;
bool windowOpening = false;

// Mode
bool bevMode = false;
bool rotateAround = false;

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  float cameraSpeed = 5.0f * deltaTime;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -=
        glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos +=
        glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraUp;
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraUp;

  // Rotations
  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    pitch += 0.5f;
  if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
    yaw += 0.5f;
  if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    roll += 0.5f;

  // Bus Movement (Simulate Driving)
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    busPos.x += sin(glm::radians(busYaw)) * busSpeed * deltaTime;
    busPos.z += cos(glm::radians(busYaw)) * busSpeed * deltaTime;
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    busPos.x -= sin(glm::radians(busYaw)) * busSpeed * deltaTime;
    busPos.z -= cos(glm::radians(busYaw)) * busSpeed * deltaTime;
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    busYaw += 50.0f * deltaTime;
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    busYaw -= 50.0f * deltaTime;

  // Toggles
  static bool lPressed = false;
  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
    if (!lPressed) {
      lightOn = !lightOn;
      lPressed = true;
    }
  } else
    lPressed = false;

  static bool gPressed = false;
  if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
    if (!gPressed) {
      fanRotating = !fanRotating;
      gPressed = true;
    }
  } else
    gPressed = false;

  static bool fPressed = false;
  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
    if (!fPressed) {
      rotateAround = !rotateAround;
      fPressed = true;
    }
  } else
    fPressed = false;

  static bool xPressed = false;
  if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
    if (!xPressed) {
      bevMode = !bevMode;
      xPressed = true;
    }
  } else
    xPressed = false;

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

const char *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aNormal;\n"
    "out vec3 FragPos; out vec3 Normal;\n"
    "uniform mat4 model; uniform mat4 view; uniform mat4 projection;\n"
    "void main() {\n"
    "   FragPos = vec3(model * vec4(aPos, 1.0));\n"
    "   Normal = mat3(transpose(inverse(model))) * aNormal;\n"
    "   gl_Position = projection * view * vec4(FragPos, 1.0);\n"
    "}\0";

const char *fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor; in vec3 Normal; in vec3 FragPos;\n"
    "uniform vec3 lightPos; uniform vec3 viewPos; uniform vec3 lightColor; "
    "uniform vec3 objectColor; uniform bool lightOn;\n"
    "void main() {\n"
    "   vec3 ambient = 0.3 * lightColor;\n"
    "   vec3 norm = normalize(Normal);\n"
    "   vec3 lightDir = normalize(lightPos - FragPos);\n"
    "   float diff = max(dot(norm, lightDir), 0.0);\n"
    "   vec3 diffuse = diff * lightColor;\n"
    "   vec3 result = (ambient + diffuse) * objectColor;\n"
    "   if(!lightOn) result *= 0.2;\n"
    "   FragColor = vec4(result, 1.0);\n"
    "}\n\0";

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window =
      glfwCreateWindow(800, 600, "3D Bus Assignment", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glewInit();
  glEnable(GL_DEPTH_TEST);

  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  Cube busBody(glm::vec3(0.8f, 0.8f, 0.8f));
  Sphere wheel(glm::vec3(0.1f, 0.1f, 0.1f));
  Cube windowPane(glm::vec3(0.0f, 0.5f, 0.8f));
  Cube door(glm::vec3(0.5f, 0.3f, 0.1f));
  Cube fan(glm::vec3(1.0f, 1.0f, 0.0f));
  Cube windshield(glm::vec3(0.0f, 0.7f, 0.9f));

  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // Camera
    glm::mat4 projection =
        glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 view;
    if (bevMode) {
      view = glm::lookAt(busPos + glm::vec3(0.0f, 15.0f, 0.1f), busPos,
                         glm::vec3(0.0f, 1.0f, 0.0f));
    } else if (rotateAround) {
      float radius = 10.0f;
      float camX = sin(glfwGetTime()) * radius;
      float camZ = cos(glfwGetTime()) * radius;
      view = glm::lookAt(busPos + glm::vec3(camX, 5.0f, camZ), busPos,
                         glm::vec3(0.0f, 1.0f, 0.0f));
    } else {
      glm::vec3 direction;
      direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
      direction.y = sin(glm::radians(pitch));
      direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
      cameraFront = glm::normalize(direction);

      // Simplified orientation
      view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
      view = glm::rotate(view, glm::radians(roll), cameraFront);
    }

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1,
                       GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE,
                       &view[0][0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1,
                 &glm::vec3(5.0f, 5.0f, 5.0f)[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1,
                 &glm::vec3(1.0f, 1.0f, 1.0f)[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1,
                 &cameraPos[0]);
    glUniform1i(glGetUniformLocation(shaderProgram, "lightOn"), (int)lightOn);

    // Render Bus
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, busPos);
    model = glm::rotate(model, glm::radians(busYaw), glm::vec3(0, 1, 0));

    // Body
    glm::mat4 bodyModel = glm::scale(model, glm::vec3(2.0f, 1.5f, 5.0f));
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1,
                 &glm::vec3(0.8f, 0.8f, 0.8f)[0]);
    busBody.draw(shaderProgram, bodyModel);

    // Windshield (Front Glass)
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
                 &glm::vec3(0.1f, 0.1f, 0.1f)[0]);
    for (int i = 0; i < 4; i++) {
      glm::mat4 wM = glm::translate(model, wheelSpecs[i]);
      wM = glm::rotate(wM, glm::radians(90.0f),
                       glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate to face outward
      wM = glm::scale(wM, glm::vec3(0.6f, 0.3f, 0.6f));
      wheel.draw(shaderProgram, wM);
    }

    // Door (Animating)
    if (doorOpening && doorOpen < 1.0f)
      doorOpen += deltaTime;
    if (!doorOpening && doorOpen > 0.0f)
      doorOpen -= deltaTime;
    glm::mat4 dM = glm::translate(model, glm::vec3(1.01f, -0.2f, 1.5f));
    dM = glm::translate(dM,
                        glm::vec3(0.0f, 0.0f, doorOpen * -0.8f)); // Slide open
    dM = glm::scale(dM, glm::vec3(0.05f, 1.0f, 0.8f));
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1,
                 &glm::vec3(0.5f, 0.3f, 0.1f)[0]);
    door.draw(shaderProgram, dM);

    // Windows (Animating)
    if (windowOpening && windowOpen < 1.0f)
      windowOpen += deltaTime;
    if (!windowOpening && windowOpen > 0.0f)
      windowOpen -= deltaTime;

    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1,
                 &glm::vec3(0.0f, 0.5f, 0.8f)[0]);
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

    // Fan (Animating)
    if (fanRotating)
      fanAngle += 500.0f * deltaTime;
    glm::mat4 fM = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));
    fM = glm::rotate(fM, glm::radians(fanAngle), glm::vec3(0, 1, 0));
    glm::mat4 fM1 = glm::scale(fM, glm::vec3(0.5f, 0.05f, 0.05f));
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1,
                 &glm::vec3(1.0f, 1.0f, 0.0f)[0]);
    fan.draw(shaderProgram, fM1);
    glm::mat4 fM2 = glm::rotate(fM, glm::radians(90.0f), glm::vec3(0, 1, 0));
    fM2 = glm::scale(fM2, glm::vec3(0.5f, 0.05f, 0.05f));
    fan.draw(shaderProgram, fM2);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
