
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cctype>
#include <cmath>
#include <iostream>
#include <string>
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

// External State
bool inExterior = true;

// Camera
Camera camera(glm::vec3(0.0f, 2.0f, 75.0f));
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
bool multiViewportMode = false;

bool sarcophagusOpen = false;
float sarcophagusSlide = 0.0f;
bool sarcophagusInteract = false;

// Lighting States
bool flashlightOn = true;
bool lanternsOn = true;

// Texture States
bool texturesEnabled = true;

// === TWEAKABLE GRAPHICS CONCEPT STATES ===
// Category 1: Geometry & Modeling
float bezierAmplitude = 1.0f;       // Bezier wave amplitude multiplier
int   fractalDepth = 4;             // Fractal tree recursion depth (1-6)

// Category 2: Texturing
// (texturesEnabled already handles simple texture toggle via T)
// Fragment blend is always active on water; vertex blend is intrinsic

// Category 3: Lighting Sources
bool directionalLightOn = true;     // Sun / directional exterior light
bool pointLightsOn = true;          // Lantern point lights
// (flashlightOn already handles spot light toggle via F)

// Category 4: Shading & Illumination
bool ambientOn   = true;            // Ambient component
bool diffuseOn   = true;            // Diffuse component
bool specularOn  = true;            // Specular component
float specularPower = 32.0f;        // Phong shininess exponent
bool  useGouraud = false;           // Gouraud vs Phong shading mode
float spotConeAngle = 14.0f;        // Spotlight inner cone angle (degrees)

const float CAMERA_EYE_HEIGHT = 1.5f;
const float CAMERA_MIN_X = -18.2f;
const float CAMERA_MAX_X = 4.6f;
const float CAMERA_MIN_Z = -52.0f;
const float CAMERA_MAX_Z = -0.8f;

const glm::vec3 SIDE_DOOR_HINT_POS(-5.0f, CAMERA_EYE_HEIGHT, -47.5f);
const float SIDE_DOOR_HINT_RADIUS = 2.8f;
const std::string SIDE_DOOR_CODE = "DHARAGOL";

bool sideDoorUnlocked = false;
bool sideDoorPlayerNearby = false;
bool sideDoorHintShown = false;
std::string sideDoorInputBuffer;
float sideDoorOpenAmount = 0.0f;
bool sideDoorZoneMuted = false;
bool horrorTrackStarted = false;
bool audioEnabled = false;

// Lighting
glm::vec3 lightPos(0.0f, 2.0f, 0.0f); // Central light

// Function prototypes
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void char_callback(GLFWwindow *window, unsigned int codepoint);
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void constrainCameraToTomb();

void drawSarcophagus(Shader &shader, Cube &cube, glm::mat4 parentModel,
                     float slideAmount, unsigned int textureID, bool useTexture);
void drawSecondRoomBurialSet(Shader &shader, Cube &cube, Cylinder &cyl,
                             glm::vec3 center, unsigned int stoneTexture,
                             unsigned int ornamentTexture, bool useTexture);
void drawLantern(Shader &shader, Cube &cube, Cylinder &cyl, glm::mat4 model,
                 float time, unsigned int textureID, bool useTexture);
void drawCamel(Shader &shader, Cube &cube, Cylinder &cyl, glm::mat4 rootModel,
               float walkPhase, unsigned int textureID, bool useTexture);
void drawFractalDesertTree(Shader &shader, Cylinder &cyl, Cube &cube,
                           glm::vec3 position, float scale, float yawDeg,
                           float seed, unsigned int barkTexture,
                           unsigned int canopyTexture, bool useTexture);
void drawDatePalmFrond(Shader &shader, Cube &cube, glm::mat4 crownModel,
                       float yawDeg, float tiltDeg, float length, float width,
                       float curveDeg, float dryFactor, unsigned int canopyTexture,
                       bool useTexture);
float pseudoNoise01(float value);

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

const LanternInfo secondRoomLanterns[] = {
    {{-18.75f, 2.2f, -45.0f}, 1.0f}, {{-18.75f, 2.2f, -50.0f}, 1.0f},
    {{-5.25f, 2.2f, -45.0f}, -1.0f}, {{-5.25f, 2.2f, -50.0f}, -1.0f},
};
const int NUM_SECOND_ROOM_LANTERNS = 4;

// Camel data: each camel walks a circular path in the desert
struct CamelInfo {
  glm::vec3 center;     // Center of circular path
  float radius;         // Radius of circular path
  float speed;          // Angular speed (radians per second)
  float phaseOffset;    // Starting angle offset
  float scale;          // Size variation
};

const CamelInfo camels[] = {
  // Spread around the island perimeter (radius ~180-230 from center)
  {{-180.0f, 0.0f,  100.0f}, 14.0f, 0.06f, 0.0f, 2.7f},   // NW
  {{ 160.0f, 0.0f,  140.0f}, 12.0f, 0.05f, 1.8f, 2.5f},   // NE
  {{   0.0f, 0.0f,  210.0f}, 13.0f, 0.06f, 3.2f, 2.6f},   // N
  {{-140.0f, 0.0f, -150.0f}, 11.0f, 0.05f, 4.6f, 2.4f},   // SW
  {{ 190.0f, 0.0f,  -80.0f}, 13.0f, 0.06f, 2.7f, 2.8f},   // E
  {{ -40.0f, 0.0f, -200.0f}, 12.0f, 0.05f, 5.1f, 2.6f},   // S
  {{ 120.0f, 0.0f,  190.0f}, 10.0f, 0.06f, 0.9f, 2.4f},   // NNE
  {{-200.0f, 0.0f,  -40.0f}, 11.0f, 0.05f, 3.8f, 2.5f},   // W
  {{ 100.0f, 0.0f, -180.0f}, 13.0f, 0.06f, 2.2f, 2.7f},   // SE
  {{-100.0f, 0.0f,  180.0f}, 12.0f, 0.05f, 4.0f, 2.5f},   // NNW
};
const int NUM_CAMELS = 10;

struct DesertTreeInfo {
  glm::vec3 position;
  float scale;
  float yawDeg;
  float seed;
};

const DesertTreeInfo desertTrees[] = {
  // Spread around the island perimeter for scenic views from pyramid top
  {{-200.0f, 0.0f,   60.0f}, 6.0f, -18.0f, 0.71f},   // W
  {{ 190.0f, 0.0f,   90.0f}, 5.5f,  24.0f, 1.43f},   // E
  {{ -60.0f, 0.0f,  210.0f}, 5.8f, -31.0f, 2.31f},   // N
  {{  80.0f, 0.0f,  200.0f}, 6.2f,  12.0f, 3.02f},   // NE
  {{-160.0f, 0.0f, -130.0f}, 5.6f, -27.0f, 3.86f},   // SW
  {{ 170.0f, 0.0f, -120.0f}, 6.1f,  36.0f, 4.42f},   // SE
  {{ -20.0f, 0.0f, -215.0f}, 5.4f,  -8.0f, 5.10f},   // S
  {{-210.0f, 0.0f,  -30.0f}, 5.9f,  42.0f, 5.82f},   // WSW
  {{ 210.0f, 0.0f,  -20.0f}, 5.7f, -15.0f, 6.33f},   // ESE
  {{-130.0f, 0.0f,  175.0f}, 6.0f,  28.0f, 0.22f},   // NNW
  {{ 140.0f, 0.0f,  160.0f}, 5.3f, -40.0f, 1.05f},   // NNE
  {{ 50.0f, 0.0f, -210.0f},  5.8f,  18.0f, 2.77f},   // SSE
  {{-190.0f, 0.0f,  130.0f}, 6.3f, -22.0f, 3.50f},   // WNW
  {{ 200.0f, 0.0f,   40.0f}, 5.5f,  33.0f, 4.18f},   // ENE
};
const int NUM_DESERT_TREES = 14;

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
  glfwSetCharCallback(window, char_callback);
  glfwSetKeyCallback(window, key_callback);

  // tell GLFW to capture our mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // glew init
  if (glewInit() != GLEW_OK) {
    std::cout << "Failed to initialize GLEW" << std::endl;
    return -1;
  }

  // configure global opengl state
  glEnable(GL_DEPTH_TEST);

  // Audio starts muted by default. Press M to toggle music.

  // build and compile shaders
  Shader mainShader("vshader.glsl", "fshader.glsl");

  // Geometry
  Cube cube;
  Cylinder cylinder(36);
  Sphere skydome(48, 96);  // High-res inverted sphere for realistic sky
  Disk sandIsland(72);     // Circular ground plane for the island

  // Load textures
  unsigned int wallTexture = loadTexture("resources/wall_texture.png");
  unsigned int floorTexture = loadTexture("resources/floor_texture.png");
  unsigned int pillarTexture = loadTexture("resources/pillar_texture.png");
  unsigned int doorTexture = loadTexture("resources/door_texture.png");
  unsigned int lanternTexture = loadTexture("resources/lantern_texture.png");
  unsigned int graveyardTexture =
      loadTexture("resources/graveyard_texture.png");
  unsigned int sandTexture = loadTexture("resources/sand_texture.png");
  unsigned int pyramidTexture = loadTexture("resources/pyramid_texture.png");
  unsigned int skyTexture = loadTexture("resources/sky_texture_hires.jpg");
  unsigned int camelTexture = loadTexture("resources/camel_texture.png");
  unsigned int treeBarkTexture = loadTexture("resources/tree_bark_texture.jpg");
  unsigned int treeCanopyTexture =
      loadTexture("resources/tree_dry_canopy_texture.jpg");
  unsigned int waterTexture = loadTexture("resources/water_texture_hd.jpg");
  unsigned int metalTexture = loadTexture("resources/rusted_metal_texture.png");

  // Shader config
  mainShader.use();
  mainShader.setInt("texture1", 0);
  mainShader.setInt("normalMap", 1);
  mainShader.setBool("useEmissive", false);
  mainShader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));
  mainShader.setVec2("uvOffset", glm::vec2(0.0f, 0.0f));
  mainShader.setBool("rotateUV90", false);
  mainShader.setBool("useWaterSurface", false);
  mainShader.setFloat("waterTime", 0.0f);
  mainShader.setFloat("waterNearZ", -170.0f);
  mainShader.setFloat("waterFarZ", -550.0f);

  // Render loop
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    // Logic
    if (bladeActive) {
      bladeTime += deltaTime;
    }
    if (sarcophagusInteract) {
      if (sarcophagusOpen && sarcophagusSlide < 2.5f)
        sarcophagusSlide += deltaTime;
      if (!sarcophagusOpen && sarcophagusSlide > 0.0f)
        sarcophagusSlide -= deltaTime;
    }

    float targetDoorOpen = sideDoorUnlocked ? 1.0f : 0.0f;
    float doorSpeed = 1.8f;
    if (sideDoorOpenAmount < targetDoorOpen) {
      sideDoorOpenAmount =
          glm::min(targetDoorOpen, sideDoorOpenAmount + doorSpeed * deltaTime);
    } else if (sideDoorOpenAmount > targetDoorOpen) {
      sideDoorOpenAmount =
          glm::max(targetDoorOpen, sideDoorOpenAmount - doorSpeed * deltaTime);
    }

    if (inExterior) {
      glClearColor(0.85f, 0.45f, 0.2f, 1.0f); // Sunset sky
    } else {
      glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mainShader.use();

    // View/Proj
    auto drawScene = [&](const glm::mat4& viewOutput, const glm::mat4& projOutput, const glm::vec3& camPosOutput, bool isOrtho) {
        mainShader.setMat4("projection", projOutput);
        mainShader.setMat4("view", viewOutput);
        mainShader.setVec3("viewPos", camPosOutput);
        mainShader.setBool("isDevOrtho", isOrtho);

    // ========== LIGHTING ==========
    int lightIndex = 0;
    if (inExterior) {
      if (directionalLightOn) {
        std::string prefix = "pointLights[0]";
        mainShader.setVec3(prefix + ".position", glm::vec3(50.0f, 200.0f, 40.0f)); // Sun position
        mainShader.setVec3(prefix + ".ambient", 0.6f, 0.5f, 0.4f);
        mainShader.setVec3(prefix + ".diffuse", 1.2f, 0.9f, 0.7f);
        mainShader.setVec3(prefix + ".specular", 0.5f, 0.4f, 0.3f);
        mainShader.setFloat(prefix + ".constant", 1.0f);
        mainShader.setFloat(prefix + ".linear", 0.0001f);
        mainShader.setFloat(prefix + ".quadratic", 0.0000001f);

        // Fill light from opposite side so shadow faces show texture
        std::string fill = "pointLights[1]";
        mainShader.setVec3(fill + ".position", glm::vec3(-150.0f, 100.0f, -100.0f));
        mainShader.setVec3(fill + ".ambient", 0.15f, 0.12f, 0.1f);
        mainShader.setVec3(fill + ".diffuse", 0.5f, 0.4f, 0.35f);
        mainShader.setVec3(fill + ".specular", 0.1f, 0.1f, 0.1f);
        mainShader.setFloat(fill + ".constant", 1.0f);
        mainShader.setFloat(fill + ".linear", 0.0001f);
        mainShader.setFloat(fill + ".quadratic", 0.0000001f);

        lightIndex = 2;
      }
    } else {
    if (pointLightsOn) {
    // Lantern point lights with warm fire color (corridor + second room)
    for (int i = 0; i < NUM_LANTERNS; i++, lightIndex++) {
      std::string prefix = "pointLights[" + std::to_string(lightIndex) + "]";
      // Slight flicker effect
      float flicker =
          0.9f + 0.1f * sin(currentFrame * 8.0f + lightIndex * 1.7f);
      mainShader.setVec3(prefix + ".position",
                         lanterns[i].position +
                             glm::vec3(lanterns[i].facingX * 0.3f, 0.3f, 0.0f));
      
      if (lanternsOn) {
        mainShader.setVec3(prefix + ".ambient", 0.06f, 0.04f, 0.02f);
        mainShader.setVec3(prefix + ".diffuse", 1.0f * flicker, 0.55f * flicker,
                           0.15f * flicker);
        mainShader.setVec3(prefix + ".specular", 0.6f, 0.4f, 0.1f);
      } else {
        mainShader.setVec3(prefix + ".ambient", 0.0f, 0.0f, 0.0f);
        mainShader.setVec3(prefix + ".diffuse", 0.0f, 0.0f, 0.0f);
        mainShader.setVec3(prefix + ".specular", 0.0f, 0.0f, 0.0f);
      }
      mainShader.setFloat(prefix + ".constant", 1.0f);
      mainShader.setFloat(prefix + ".linear", 0.22f); // Sharper falloff
      mainShader.setFloat(prefix + ".quadratic", 0.12f);
    }

    for (int i = 0; i < NUM_SECOND_ROOM_LANTERNS; i++, lightIndex++) {
      std::string prefix = "pointLights[" + std::to_string(lightIndex) + "]";
      float flicker =
          0.9f + 0.1f * sin(currentFrame * 8.0f + lightIndex * 1.7f);
      mainShader.setVec3(
          prefix + ".position",
          secondRoomLanterns[i].position +
              glm::vec3(secondRoomLanterns[i].facingX * 0.3f, 0.3f, 0.0f));

      if (lanternsOn) {
        mainShader.setVec3(prefix + ".ambient", 0.06f, 0.04f, 0.02f);
        mainShader.setVec3(prefix + ".diffuse", 1.0f * flicker,
                           0.55f * flicker, 0.15f * flicker);
        mainShader.setVec3(prefix + ".specular", 0.6f, 0.4f, 0.1f);
      } else {
        mainShader.setVec3(prefix + ".ambient", 0.0f, 0.0f, 0.0f);
        mainShader.setVec3(prefix + ".diffuse", 0.0f, 0.0f, 0.0f);
        mainShader.setVec3(prefix + ".specular", 0.0f, 0.0f, 0.0f);
      }
      mainShader.setFloat(prefix + ".constant", 1.0f);
      mainShader.setFloat(prefix + ".linear", 0.22f);
      mainShader.setFloat(prefix + ".quadratic", 0.12f);
    }

    // Focused fill for the second-room burial display.
    {
      std::string prefix = "pointLights[" + std::to_string(lightIndex) + "]";
      mainShader.setVec3(prefix + ".position", glm::vec3(-11.2f, 1.65f, -47.1f));
      if (lanternsOn) {
        mainShader.setVec3(prefix + ".ambient", 0.04f, 0.03f, 0.02f);
        mainShader.setVec3(prefix + ".diffuse", 0.62f, 0.46f, 0.30f);
        mainShader.setVec3(prefix + ".specular", 0.26f, 0.20f, 0.12f);
      } else {
        mainShader.setVec3(prefix + ".ambient", 0.0f, 0.0f, 0.0f);
        mainShader.setVec3(prefix + ".diffuse", 0.0f, 0.0f, 0.0f);
        mainShader.setVec3(prefix + ".specular", 0.0f, 0.0f, 0.0f);
      }
      mainShader.setFloat(prefix + ".constant", 1.0f);
      mainShader.setFloat(prefix + ".linear", 0.18f);
      mainShader.setFloat(prefix + ".quadratic", 0.08f);
      lightIndex++;
    }
    } // end pointLightsOn
    }

    mainShader.setInt("numPointLights", lightIndex);

    // SpotLight (Flashlight) – dim for atmosphere
    mainShader.setVec3("spotLight.position", camera.Position);
    mainShader.setVec3("spotLight.direction", camera.Front);
    
    if (flashlightOn) {
      mainShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
      mainShader.setVec3("spotLight.diffuse", 0.4f, 0.35f, 0.25f); // Dim warm
      mainShader.setVec3("spotLight.specular", 0.3f, 0.3f, 0.3f);
    } else {
      mainShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
      mainShader.setVec3("spotLight.diffuse", 0.0f, 0.0f, 0.0f); 
      mainShader.setVec3("spotLight.specular", 0.0f, 0.0f, 0.0f);
    }
    
    mainShader.setFloat("spotLight.constant", 1.0f);
    mainShader.setFloat("spotLight.linear", 0.14f);
    mainShader.setFloat("spotLight.quadratic", 0.07f);
    mainShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(spotConeAngle)));
    mainShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(spotConeAngle + 4.0f)));
    mainShader.setBool("spotLightOn", flashlightOn);

    // === TWEAKABLE SHADER UNIFORMS ===
    mainShader.setBool("ambientOn", ambientOn);
    mainShader.setBool("diffuseOn", diffuseOn);
    mainShader.setBool("specularOn", specularOn);
    mainShader.setFloat("specularPower", specularPower);
    mainShader.setBool("useGouraud", useGouraud);

    // Gouraud vertex shader uniforms (use primary light source position)
    if (inExterior) {
      mainShader.setVec3("gouraudLightPos", glm::vec3(50.0f, 200.0f, 40.0f));
      mainShader.setVec3("gouraudLightColor", glm::vec3(1.2f, 0.9f, 0.7f));
    } else {
      // Use the first lantern as the Gouraud reference light
      mainShader.setVec3("gouraudLightPos", lanterns[0].position);
      mainShader.setVec3("gouraudLightColor", glm::vec3(1.0f, 0.55f, 0.15f));
    }
    mainShader.setVec3("gouraudViewPos", camPosOutput);

    // ========== DRAW SCENE ==========
    mainShader.setBool("useEmissive", false);
    mainShader.setFloat("emissiveBrightness", 1.0f);
    mainShader.setBool("useNormalMap", false);
    mainShader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));
    mainShader.setVec2("uvOffset", glm::vec2(0.0f, 0.0f));
    mainShader.setBool("rotateUV90", false);
    mainShader.setBool("useWaterSurface", false);
    mainShader.setFloat("waterTime", currentFrame);
    mainShader.setFloat("bezierAmplitude", bezierAmplitude);

    if (inExterior) {
      // Background Skydome (inverted sphere — no corner seams)
      glDepthMask(GL_FALSE);  // Don't write depth so everything draws in front
      mainShader.setBool("useEmissive", true);
      // Dim the sky dramatically if the sun is turned off
      mainShader.setFloat("emissiveBrightness", directionalLightOn ? 1.0f : 0.05f);
      mainShader.setBool("useTexture", true);
      glBindTexture(GL_TEXTURE_2D, skyTexture);
      glm::mat4 model = glm::mat4(1.0f);
      // Center the skydome on the camera so the sky appears at infinite distance
      model = glm::translate(model, camera.Position);
      model = glm::scale(model, glm::vec3(600.0f, 600.0f, 600.0f));
      mainShader.setMat4("model", model);
      // Sphere UV mapping is equirectangular — use full texture, no half-image hack
      mainShader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));
      mainShader.setVec2("uvOffset", glm::vec2(0.0f, 0.0f));
      skydome.draw(mainShader.ID);
      glDepthMask(GL_TRUE);   // Restore depth writes for the rest of the scene
      
      // Revert states
      mainShader.setBool("useEmissive", false);
      mainShader.setFloat("emissiveBrightness", 1.0f);
      mainShader.setBool("useTexture", texturesEnabled);

      // Sand Ground — circular island plateau above the flood water
      glBindTexture(GL_TEXTURE_2D, sandTexture);
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(0.0f, -0.02f, 0.0f));
      // Disk radius is 0.5, so scale 500 gives radius 250 (covers all pyramids with margin)
      model = glm::scale(model, glm::vec3(500.0f, 1.0f, 500.0f));
      mainShader.setMat4("model", model);
      mainShader.setVec3("objectColor", 0.6f, 0.5f, 0.4f);
      mainShader.setVec2("uvScale", glm::vec2(30.0f, 30.0f));
      sandIsland.draw(mainShader.ID);

      // === NILE FLOOD — infinite ocean surrounding the island ===
      // Water sits just below the sand so it's visible all around the island edges.
      glBindTexture(GL_TEXTURE_2D, waterTexture);
      mainShader.setBool("useWaterSurface", true);
      // waterNearZ = inner shore radius, waterFarZ = deep ocean outer radius
      mainShader.setFloat("waterNearZ", 255.0f);  // match island disk radius (250) + margin
      mainShader.setFloat("waterFarZ", 900.0f);
      mainShader.setVec3("objectColor", 1.0f, 1.0f, 1.0f);

      // Single massive water plane covering the entire world.
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(0.0f, -0.08f, 0.0f));
      model = glm::scale(model, glm::vec3(1800.0f, 0.004f, 1800.0f));
      mainShader.setMat4("model", model);
      mainShader.setVec2("uvScale", glm::vec2(60.0f, 60.0f));
      mainShader.setVec2("uvOffset", glm::vec2(currentFrame * 0.0012f,
                     currentFrame * 0.0008f));
      cube.draw(mainShader.ID);

      mainShader.setBool("useWaterSurface", false);
      mainShader.setVec2("uvOffset", glm::vec2(0.0f, 0.0f));

      // Pyramid Geometry - draw each step as separate face panels
      glActiveTexture(GL_TEXTURE0);
      mainShader.setVec3("objectColor", 0.85f, 0.75f, 0.65f);
      const float tileSize = 2.0f;
      const float heightPerStep = 1.0f;
      struct PyramidLayout {
        float centerX;
        float centerZ;
        float baseSize;
        int numSteps;
      };
      const PyramidLayout pyramids[] = {
          {0.0f, -15.0f, 90.0f, 45},     // Main pyramid (center)
          {-86.0f, -34.0f, 66.0f, 33},   // Second pyramid (left/back), fully separate
          {92.0f, -42.0f, 74.0f, 37},    // Third pyramid (right/back), fully separate
      };

      auto drawSteppedPyramid = [&](float centerX, float centerZ, float baseSize, int numSteps) {
        float stepShrink = baseSize / static_cast<float>(numSteps);
        for (int i = 0; i < numSteps; i++) {
          float size = baseSize - i * stepShrink;
          float yPos = i * heightPerStep + heightPerStep * 0.5f;
          float halfSize = size * 0.5f;

          glBindTexture(GL_TEXTURE_2D, pyramidTexture);
          mainShader.setVec2("uvScale", glm::vec2(size / tileSize, heightPerStep / tileSize));

          // Front face (Z+ side)
          model = glm::mat4(1.0f);
          model = glm::translate(model, glm::vec3(centerX, yPos, centerZ + halfSize));
          model = glm::scale(model, glm::vec3(size, heightPerStep, 0.1f));
          mainShader.setMat4("model", model);
          cube.draw(mainShader.ID);

          // Back face (Z- side)
          model = glm::mat4(1.0f);
          model = glm::translate(model, glm::vec3(centerX, yPos, centerZ - halfSize));
          model = glm::scale(model, glm::vec3(size, heightPerStep, 0.1f));
          mainShader.setMat4("model", model);
          cube.draw(mainShader.ID);

          // Left face (X- side) - rotated so front face points left
          model = glm::mat4(1.0f);
          model = glm::translate(model, glm::vec3(centerX - halfSize, yPos, centerZ));
          model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
          model = glm::scale(model, glm::vec3(size, heightPerStep, 0.1f));
          mainShader.setMat4("model", model);
          cube.draw(mainShader.ID);

          // Right face (X+ side) - rotated so front face points right
          model = glm::mat4(1.0f);
          model = glm::translate(model, glm::vec3(centerX + halfSize, yPos, centerZ));
          model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
          model = glm::scale(model, glm::vec3(size, heightPerStep, 0.1f));
          mainShader.setMat4("model", model);
          cube.draw(mainShader.ID);

          // Top face (step ledge)
          model = glm::mat4(1.0f);
          model = glm::translate(model, glm::vec3(centerX, yPos + heightPerStep * 0.5f, centerZ));
          model = glm::scale(model, glm::vec3(size, 0.1f, size));
          mainShader.setMat4("model", model);
          mainShader.setVec2("uvScale", glm::vec2(size / tileSize, size / tileSize));
          cube.draw(mainShader.ID);
        }
      };

      for (const PyramidLayout &pyramid : pyramids) {
        drawSteppedPyramid(pyramid.centerX, pyramid.centerZ,
                           pyramid.baseSize, pyramid.numSteps);
      }

      // Entrance Vestibule
      float vestZ = 31.5f;
      
      // Left pillar block
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(-2.8f, 2.5f, vestZ));
      model = glm::scale(model, glm::vec3(2.5f, 5.0f, 3.0f));
      mainShader.setMat4("model", model);
      mainShader.setVec2("uvScale", glm::vec2(1.0f, 2.0f));
      cube.draw(mainShader.ID);

      // Right pillar block
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(2.8f, 2.5f, vestZ));
      model = glm::scale(model, glm::vec3(2.5f, 5.0f, 3.0f));
      mainShader.setMat4("model", model);
      mainShader.setVec2("uvScale", glm::vec2(1.0f, 2.0f));
      cube.draw(mainShader.ID);

      // Top block
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(0.0f, 5.5f, vestZ));
      model = glm::scale(model, glm::vec3(8.1f, 1.0f, 3.0f));
      mainShader.setMat4("model", model);
      mainShader.setVec2("uvScale", glm::vec2(3.0f, 0.5f));
      cube.draw(mainShader.ID);

      // Doorway with door texture
      mainShader.setBool("useTexture", texturesEnabled);
      glBindTexture(GL_TEXTURE_2D, doorTexture);
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(0.0f, 2.5f, vestZ - 1.25f));
      model = glm::scale(model, glm::vec3(3.1f, 5.0f, 0.5f));
      mainShader.setMat4("model", model);
      mainShader.setVec3("objectColor", 0.7f, 0.6f, 0.5f);
      mainShader.setVec2("uvScale", glm::vec2(1.0f, -1.0f));
      mainShader.setVec2("uvOffset", glm::vec2(0.0f, 1.0f));
      cube.draw(mainShader.ID);

      // Reset UV for camels
      mainShader.setVec2("uvOffset", glm::vec2(0.0f, 0.0f));
      mainShader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));

      // === CAMELS ===
      const float minCamelToPyramidPadding = 25.0f;
      const float minCamelToPlayer = 42.0f;
      for (int i = 0; i < NUM_CAMELS; i++) {
        float angle = currentFrame * camels[i].speed + camels[i].phaseOffset;
        float cx = camels[i].center.x + cos(angle) * camels[i].radius;
        float cz = camels[i].center.z + sin(angle) * camels[i].radius;

        glm::vec3 camelPos(cx, 0.0f, cz);
        bool tooCloseToAnyPyramid = false;
        for (const PyramidLayout &pyramid : pyramids) {
          glm::vec3 pyramidCenter(pyramid.centerX, 0.0f, pyramid.centerZ);
          float minCamelToPyramid = pyramid.baseSize * 0.5f + minCamelToPyramidPadding;
          if (glm::distance(camelPos, pyramidCenter) < minCamelToPyramid) {
            tooCloseToAnyPyramid = true;
            break;
          }
        }
        if (tooCloseToAnyPyramid) {
          continue;
        }
        if (glm::distance(camelPos, camera.Position) < minCamelToPlayer) {
          continue;
        }

        // Face direction of travel (tangent to circle)
        float facingAngle = angle + 3.14159f * 0.5f;

        // Walk phase based on distance traveled
        float walkPhase = currentFrame * camels[i].speed * camels[i].radius * 1.2f + camels[i].phaseOffset;

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(cx, 0.0f, cz));
        model = glm::rotate(model, facingAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(camels[i].scale));
        drawCamel(mainShader, cube, cylinder, model, walkPhase, camelTexture, texturesEnabled);
      }

      // === FRACTAL DESERT TREES ===
      for (int i = 0; i < NUM_DESERT_TREES; i++) {
        drawFractalDesertTree(mainShader, cylinder, cube, desertTrees[i].position,
                              desertTrees[i].scale, desertTrees[i].yawDeg,
                              desertTrees[i].seed, treeBarkTexture,
                              treeCanopyTexture, texturesEnabled);
      }

    } else {

    // Draw Floor (Continuous)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -1.0f, -25.0f));
    model = glm::scale(model, glm::vec3(10.0f, 0.1f, 50.0f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.6f, 0.55f, 0.5f);
    mainShader.setBool("useTexture", texturesEnabled);
    mainShader.setVec2("uvScale", glm::vec2(5.0f, 25.0f));
    cube.draw(mainShader.ID);

    // Segmented Walls, Ceiling, and Dividers
    for (int i = 0; i < 10; i++) {
      float zPos = -i * 5.0f;

      // --- 1. Vertical Dividers (Wall Columns) ---
      mainShader.setBool("useTexture", texturesEnabled);
      glBindTexture(GL_TEXTURE_2D, pillarTexture);
      mainShader.setVec3("objectColor", 0.65f, 0.55f, 0.4f);
      mainShader.setVec2("uvScale", glm::vec2(1.0f, 5.0f)); // Vertical grooves
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
      if (!isOrtho) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 3.85f, zPos));
        model = glm::scale(model, glm::vec3(10.0f, 0.35f, 0.5f));
        mainShader.setMat4("model", model);
        cube.draw(mainShader.ID);
      }

      // --- 3. Wall Panels (between dividers) ---
      mainShader.setBool("useTexture", texturesEnabled);
      glBindTexture(GL_TEXTURE_2D, wallTexture);
      mainShader.setVec3("objectColor", 0.7f, 0.6f, 0.4f);
      mainShader.setVec2("uvScale", glm::vec2(0.8f, 1.0f)); // Large figures

      // Left Panel
      if (i == 9) {
        // Full-height door opening in the left wall near the far end.
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-5.0f, 1.5f, -49.1f));
        model = glm::scale(model, glm::vec3(0.2f, 5.0f, 1.3f));
        mainShader.setMat4("model", model);
        cube.draw(mainShader.ID);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-5.0f, 1.5f, -45.9f));
        model = glm::scale(model, glm::vec3(0.2f, 5.0f, 1.3f));
        mainShader.setMat4("model", model);
        cube.draw(mainShader.ID);
      } else {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-5.0f, 1.5f, zPos - 2.5f));
        model = glm::scale(model, glm::vec3(0.2f, 5.0f, 4.5f));
        mainShader.setMat4("model", model);
        cube.draw(mainShader.ID);
      }
      // Right Panel
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(5.0f, 1.5f, zPos - 2.5f));
      model = glm::scale(model, glm::vec3(0.2f, 5.0f, 4.5f));
      mainShader.setMat4("model", model);
      cube.draw(mainShader.ID);

      // --- 4. Ceiling Panels (Now using floor_texture as requested) ---
      if (!isOrtho) {
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 4.05f, zPos - 2.5f));
        model = glm::scale(model, glm::vec3(10.0f, 0.1f, 4.5f));
        mainShader.setMat4("model", model);
        mainShader.setVec3("objectColor", 0.45f, 0.35f, 0.25f);
        mainShader.setVec2("uvScale", glm::vec2(2.0f, 2.0f));
        cube.draw(mainShader.ID);
      }
    }

    // === SWINGING TRAP ===
    {
      float trapZ = -15.0f; // Align with the wooden ceiling beam just before the tomb
      // Swing back and forth up to 75 degrees using sine wave
      float swingAngle = sin(bladeTime * 2.5f) * glm::radians(75.0f);

      mainShader.setBool("useTexture", texturesEnabled);
      glBindTexture(GL_TEXTURE_2D, metalTexture);
      mainShader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));
      mainShader.setVec3("objectColor", 0.5f, 0.5f, 0.5f); // Neutral tint for texture

      // 1. Ceiling Mount / Pivot Engine
      glm::mat4 mountModel = glm::mat4(1.0f);
      mountModel = glm::translate(mountModel, glm::vec3(0.0f, 3.85f, trapZ));
      mountModel = glm::scale(mountModel, glm::vec3(1.2f, 0.4f, 0.6f));
      mainShader.setMat4("model", mountModel);
      cube.draw(mainShader.ID);

      // 2. The Swinging Arm and Blade
      glm::mat4 pivot = glm::mat4(1.0f);
      pivot = glm::translate(pivot, glm::vec3(0.0f, 3.8f, trapZ));
      // Swing along X axis (across corridor) by rotating around Z axis
      pivot = glm::rotate(pivot, swingAngle, glm::vec3(0.0f, 0.0f, 1.0f));

      // Arm (Pole)
      glm::mat4 armModel = glm::translate(pivot, glm::vec3(0.0f, -1.8f, 0.0f));
      armModel = glm::scale(armModel, glm::vec3(0.15f, 3.6f, 0.15f));
      mainShader.setMat4("model", armModel);
      mainShader.setVec2("uvScale", glm::vec2(1.0f, 5.0f));
      cube.draw(mainShader.ID);

      // Heavy counterweight / central bracket at intersection of arm and blade
      glm::mat4 bladeCenter = glm::translate(pivot, glm::vec3(0.0f, -3.2f, 0.0f));
      bladeCenter = glm::scale(bladeCenter, glm::vec3(0.6f, 0.7f, 0.15f));
      mainShader.setMat4("model", bladeCenter);
      mainShader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));
      cube.draw(mainShader.ID);

      // Main Blade (Constructed via Quadratic Bezier Curve for a perfect sweeping arc)
      int numSegments = 60;
      float bladeWidth = 3.8f;
      float segmentWidth = (bladeWidth / numSegments) * 1.02f; // Slight overlap to prevent gaps

      glm::vec3 p0(-1.9f, -3.2f, 0.0f); // Left tip of the blade
      glm::vec3 p1( 0.0f, -4.8f, 0.0f); // Bezier Control point pulling the outer edge into a deep arc
      glm::vec3 p2( 1.9f, -3.2f, 0.0f); // Right tip of the blade
      
      for (int i = 0; i <= numSegments; ++i) {
          float t = (float)i / numSegments;
          float invT = 1.0f - t;
          
          // Evaluate Quadratic Bezier B(t) = (1-t)^2*P0 + 2(1-t)t*P1 + t^2*P2
          glm::vec3 pt = invT * invT * p0 + 2.0f * invT * t * p1 + t * t * p2;
          
          float topY = -3.2f;   // Flat top edge of the blade
          float bottomY = pt.y; // Curved bottom arc of the blade
          float height = topY - bottomY;
          if (height < 0.01f) height = 0.01f;
          float centerY = topY - height * 0.5f;
          
          glm::mat4 sliceModel = glm::translate(pivot, glm::vec3(pt.x, centerY, 0.0f));
          sliceModel = glm::scale(sliceModel, glm::vec3(segmentWidth, height, 0.06f));
          mainShader.setMat4("model", sliceModel);
          // Scale texture mapping so the texture flows across the blade
          mainShader.setVec2("uvScale", glm::vec2(0.2f, height)); 
          cube.draw(mainShader.ID);
      }
    }

    // Back wall remains solid; side access is now through the left wall door.
    mainShader.setBool("useTexture", texturesEnabled);
    glBindTexture(GL_TEXTURE_2D, wallTexture);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 1.5f, -50.0f));
    model = glm::scale(model, glm::vec3(10.0f, 5.0f, 0.2f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.7f, 0.6f, 0.4f);
    mainShader.setVec2("uvScale", glm::vec2(2.0f, 1.0f));
    cube.draw(mainShader.ID);

    // Ornate full-height stone frame around the left-wall door.
    glBindTexture(GL_TEXTURE_2D, wallTexture);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 1.5f, -49.2f));
    model = glm::scale(model, glm::vec3(0.26f, 5.0f, 1.2f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.72f, 0.62f, 0.45f);
    mainShader.setVec2("uvScale", glm::vec2(0.8f, 2.0f));
    cube.draw(mainShader.ID);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 1.5f, -45.8f));
    model = glm::scale(model, glm::vec3(0.26f, 5.0f, 1.2f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.72f, 0.62f, 0.45f);
    mainShader.setVec2("uvScale", glm::vec2(0.8f, 2.0f));
    cube.draw(mainShader.ID);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 3.85f, -47.5f));
    model = glm::scale(model, glm::vec3(0.26f, 0.3f, 2.2f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.70f, 0.60f, 0.42f);
    mainShader.setVec2("uvScale", glm::vec2(1.5f, 0.6f));
    cube.draw(mainShader.ID);

    // Full-height carved door leaves (kept open for access).
    glBindTexture(GL_TEXTURE_2D, doorTexture);
    mainShader.setBool("rotateUV90", true);
    glm::mat4 leftDoorBase = glm::mat4(1.0f);
    leftDoorBase = glm::translate(leftDoorBase, glm::vec3(-5.0f, 1.5f, -47.95f));
    leftDoorBase = glm::rotate(leftDoorBase,
                   glm::radians(-68.0f * sideDoorOpenAmount),
                   glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 leftDoor = glm::scale(leftDoorBase, glm::vec3(0.14f, 5.0f, 1.05f));
    mainShader.setMat4("model", leftDoor);
    mainShader.setVec3("objectColor", 1.0f, 1.0f, 1.0f);
    mainShader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));
    mainShader.setVec2("uvOffset", glm::vec2(0.0f, 0.0f));
    cube.draw(mainShader.ID);

    glm::mat4 rightDoorBase = glm::mat4(1.0f);
    rightDoorBase = glm::translate(rightDoorBase, glm::vec3(-5.0f, 1.5f, -47.05f));
    rightDoorBase = glm::rotate(rightDoorBase,
                  glm::radians(68.0f * sideDoorOpenAmount),
                  glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 rightDoor =
      glm::scale(rightDoorBase, glm::vec3(0.14f, 5.0f, 1.05f));
    mainShader.setMat4("model", rightDoor);
    mainShader.setVec3("objectColor", 1.0f, 1.0f, 1.0f);
    mainShader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));
    mainShader.setVec2("uvOffset", glm::vec2(0.0f, 0.0f));
    cube.draw(mainShader.ID);

    // Bronze-like horizontal straps to evoke the reference tomb door style.
    mainShader.setBool("useTexture", false);
    mainShader.setVec3("objectColor", 0.28f, 0.19f, 0.10f);

    glm::mat4 leftBandTop =
      glm::translate(leftDoorBase, glm::vec3(0.04f, 1.2f, 0.0f));
    leftBandTop = glm::scale(leftBandTop, glm::vec3(0.03f, 0.12f, 1.0f));
    mainShader.setMat4("model", leftBandTop);
    cube.draw(mainShader.ID);

    glm::mat4 leftBandMid =
      glm::translate(leftDoorBase, glm::vec3(0.04f, -0.1f, 0.0f));
    leftBandMid = glm::scale(leftBandMid, glm::vec3(0.03f, 0.10f, 1.0f));
    mainShader.setMat4("model", leftBandMid);
    cube.draw(mainShader.ID);

    glm::mat4 rightBandTop =
      glm::translate(rightDoorBase, glm::vec3(-0.04f, 1.2f, 0.0f));
    rightBandTop = glm::scale(rightBandTop, glm::vec3(0.03f, 0.12f, 1.0f));
    mainShader.setMat4("model", rightBandTop);
    cube.draw(mainShader.ID);

    glm::mat4 rightBandMid =
      glm::translate(rightDoorBase, glm::vec3(-0.04f, -0.1f, 0.0f));
    rightBandMid = glm::scale(rightBandMid, glm::vec3(0.03f, 0.10f, 1.0f));
    mainShader.setMat4("model", rightBandMid);
    cube.draw(mainShader.ID);

    mainShader.setBool("useTexture", texturesEnabled);

    // Second room chamber on the left side of the main hall.
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-12.0f, -1.0f, -47.5f));
    model = glm::scale(model, glm::vec3(14.0f, 0.1f, 10.0f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.6f, 0.55f, 0.5f);
    mainShader.setVec2("uvScale", glm::vec2(7.0f, 5.0f));
    cube.draw(mainShader.ID);

      if (!isOrtho) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-12.0f, 4.05f, -47.5f));
        model = glm::scale(model, glm::vec3(14.0f, 0.1f, 10.0f));
        mainShader.setMat4("model", model);
        mainShader.setVec3("objectColor", 0.45f, 0.35f, 0.25f);
        mainShader.setVec2("uvScale", glm::vec2(3.0f, 2.5f));
        cube.draw(mainShader.ID);
      }

    glBindTexture(GL_TEXTURE_2D, wallTexture);
    // Outer left wall of side chamber
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-19.0f, 1.5f, -47.5f));
    model = glm::scale(model, glm::vec3(0.2f, 5.0f, 10.0f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.7f, 0.6f, 0.4f);
    mainShader.setVec2("uvScale", glm::vec2(1.5f, 2.0f));
    cube.draw(mainShader.ID);

    // Chamber north wall
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-12.0f, 1.5f, -52.5f));
    model = glm::scale(model, glm::vec3(14.0f, 5.0f, 0.2f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.68f, 0.58f, 0.38f);
    mainShader.setVec2("uvScale", glm::vec2(2.8f, 1.0f));
    cube.draw(mainShader.ID);

    // Seal shared boundary around the doorway to remove thin void gaps.
    // North side seal (extended to fully meet north wall)
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 1.5f, -50.5f));
    model = glm::scale(model, glm::vec3(0.2f, 5.0f, 3.8f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.7f, 0.6f, 0.4f);
    mainShader.setVec2("uvScale", glm::vec2(0.8f, 1.0f));
    cube.draw(mainShader.ID);

    // South side seal (extended to fully meet south wall)
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 1.5f, -44.5f));
    model = glm::scale(model, glm::vec3(0.2f, 5.0f, 3.8f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.7f, 0.6f, 0.4f);
    mainShader.setVec2("uvScale", glm::vec2(0.8f, 1.0f));
    cube.draw(mainShader.ID);

    // Tiny top seam cap to avoid hairline crack at the ceiling-edge junction.
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 4.03f, -47.5f));
    model = glm::scale(model, glm::vec3(0.2f, 0.06f, 2.5f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.68f, 0.58f, 0.40f);
    mainShader.setVec2("uvScale", glm::vec2(0.8f, 0.5f));
    cube.draw(mainShader.ID);

    // Chamber south wall
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-12.0f, 1.5f, -42.5f));
    model = glm::scale(model, glm::vec3(14.0f, 5.0f, 0.2f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.68f, 0.58f, 0.38f);
    mainShader.setVec2("uvScale", glm::vec2(2.8f, 1.0f));
    cube.draw(mainShader.ID);

    // Hero burial composition in the second chamber (unlocked by DHARAGOL door).
    drawSecondRoomBurialSet(mainShader, cube, cylinder,
                glm::vec3(-12.0f, -0.95f, -47.5f),
                floorTexture, graveyardTexture, texturesEnabled);

    // Front wall cap to keep the view enclosed inside the tomb
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.35f));
    model = glm::scale(model, glm::vec3(10.0f, 5.0f, 0.2f));
    mainShader.setMat4("model", model);
    mainShader.setVec3("objectColor", 0.7f, 0.6f, 0.4f);
    mainShader.setVec2("uvScale", glm::vec2(2.0f, 1.0f));
    cube.draw(mainShader.ID);
    mainShader.setBool("rotateUV90", false);

    mainShader.setBool("useTexture", false);
    mainShader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));

    // 4. Pillars removed (as requested)

    // 5. Wall-mounted Lanterns
    for (int i = 0; i < NUM_LANTERNS; i++) {
      glm::mat4 lm = glm::mat4(1.0f);
      lm = glm::translate(lm, lanterns[i].position);
      // Scale facing direction
      lm = glm::scale(lm, glm::vec3(lanterns[i].facingX, 1.0f, 1.0f));
      // Pass lanternsOn and texturesEnabled to drawLantern
      drawLantern(mainShader, cube, cylinder, lm, (lanternsOn && pointLightsOn) ? currentFrame : 0.0f, lanternTexture, texturesEnabled);
    }

    for (int i = 0; i < NUM_SECOND_ROOM_LANTERNS; i++) {
      glm::mat4 lm = glm::mat4(1.0f);
      lm = glm::translate(lm, secondRoomLanterns[i].position);
      lm = glm::scale(lm, glm::vec3(secondRoomLanterns[i].facingX, 1.0f, 1.0f));
      drawLantern(mainShader, cube, cylinder, lm,
                  (lanternsOn && pointLightsOn) ? currentFrame : 0.0f, lanternTexture,
                  texturesEnabled);
    }

    // 7. Sarcophagus (Hierarchical + Interactive)
    glm::mat4 sarcPos = glm::mat4(1.0f);
    sarcPos = glm::translate(sarcPos, glm::vec3(0.0f, -0.5f, -20.0f));
    drawSarcophagus(mainShader, cube, sarcPos, sarcophagusSlide,
                    graveyardTexture, texturesEnabled);

    }
    }; // End of drawScene lambda

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    if (!multiViewportMode) {
      glViewport(0, 0, fbWidth, fbHeight);
      glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)fbWidth / (float)fbHeight, 0.1f, 2000.0f);
      glm::mat4 view = camera.GetViewMatrix();
      drawScene(view, projection, camera.Position, false);
    } else {
      int halfW = fbWidth / 2;
      int halfH = fbHeight / 2;
      float aspect = (float)halfW / (float)halfH;
      
      // Tighten the orthographic scale so cameras act as active "room trackers" rather than full-dungeon maps
      float orthoScale = 15.0f; 
      float oW = orthoScale * aspect;
      float oH = orthoScale;

      // Top-Left: Top View (Orthographic)
      glViewport(0, halfH, halfW, halfH);
      glm::mat4 topProj = glm::ortho(-oW, oW, -oH, oH, 0.1f, 2000.0f);
      glm::vec3 topPos(camera.Position.x, 200.0f, camera.Position.z);
      glm::mat4 topView = glm::lookAt(topPos, glm::vec3(camera.Position.x, 0.0f, camera.Position.z), glm::vec3(0.0f, 0.0f, -1.0f));
      drawScene(topView, topProj, topPos, true);

      // Top-Right: Dynamic Front View (Orthographic)
      // Camera is placed ahead of the player, looking directly back into the player's face
      glViewport(halfW, halfH, halfW, halfH);
      glm::mat4 frontProj = glm::ortho(-oW, oW, -oH, oH, 0.1f, 2000.0f);
      glm::vec3 frontPos = camera.Position + camera.Front * 30.0f; // Pull camera out to view player's front
      // Look back at the player
      glm::mat4 frontView = glm::lookAt(frontPos, camera.Position, glm::vec3(0.0f, 1.0f, 0.0f));
      drawScene(frontView, frontProj, frontPos, true);

      // Bottom-Left: Isometric View (Orthographic)
      float isoScale = 22.0f; // Slightly wider for isometric map feel
      glViewport(0, 0, halfW, halfH);
      glm::mat4 isoProj = glm::ortho(-isoScale * aspect, isoScale * aspect, -isoScale, isoScale, -500.0f, 2000.0f);
      glm::vec3 isoPos = camera.Position + glm::vec3(40.0f, 40.0f, 40.0f);
      glm::mat4 isoView = glm::lookAt(isoPos, camera.Position, glm::vec3(0.0f, 1.0f, 0.0f));
      drawScene(isoView, isoProj, isoPos, true);

      // Bottom-Right: Standard Perspective View
      glViewport(halfW, 0, halfW, halfH);
      glm::mat4 perspProj = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 2000.0f);
      glm::mat4 perspView = camera.GetViewMatrix();
      drawScene(perspView, perspProj, camera.Position, false);
    }

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

  // --- New Interactions: State Trackers ---
  static bool fKeyPressed = false;
  static bool lKeyPressed = false;
  static bool tKeyPressed = false;
  static bool mKeyPressed = false;
  static bool eKeyPressed = false;
  static bool jKeyPressed = false;
  static bool kKeyPressed = false;
  static bool pKeyPressed = false;
  static bool iKeyPressed = false;
  static bool oKeyPressed = false;
  static bool hKeyPressed = false;
  static bool vKeyPressed = false;

  float doorDistance = glm::length(camera.Position - SIDE_DOOR_HINT_POS);
  sideDoorPlayerNearby = (doorDistance < SIDE_DOOR_HINT_RADIUS);

  if (sideDoorPlayerNearby && !sideDoorUnlocked && !sideDoorZoneMuted) {
    stopBackgroundMusic();
    sideDoorZoneMuted = true;
  }

  if (!sideDoorPlayerNearby && !sideDoorUnlocked && sideDoorZoneMuted &&
      !horrorTrackStarted) {
    if (audioEnabled)
      startBackgroundMusic("resources/arabian_nights.mp3");
    sideDoorZoneMuted = false;
  }

  if (sideDoorPlayerNearby && !sideDoorUnlocked && !sideDoorHintShown) {
    std::cout << "\n=== ANCIENT DOOR SEAL ===\n";
    std::cout << "Inscription: EIBSBHPM\n";
    std::cout << "Hint: The code is Caesar-shifted by +1."
                 " Shift each letter back by 1 and press ENTER.\n";
    std::cout << "Type your answer with keyboard letters (A-Z)."
                 " Backspace edits.\n";
    std::cout << "========================\n";
    sideDoorHintShown = true;
  }

  if (!sideDoorPlayerNearby) {
    sideDoorHintShown = false;
    sideDoorInputBuffer.clear();
  }

  bool passwordModeActive = sideDoorPlayerNearby && !sideDoorUnlocked;
  if (passwordModeActive) {
    // In password mode, only text input callbacks should be active.
    fKeyPressed = false;
    lKeyPressed = false;
    tKeyPressed = false;
    eKeyPressed = false;
    constrainCameraToTomb();
    return;
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, deltaTime);

  // Vertical movement (fly up/down) — exterior only
  if (inExterior) {
    float flySpeed = camera.MovementSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
      camera.Position.y += flySpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
      camera.Position.y -= flySpeed;
  }

  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
    if (!fKeyPressed) {
      flashlightOn = !flashlightOn; // Toggle
      fKeyPressed = true;
    }
  } else {
    fKeyPressed = false;
  }

  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
    if (!lKeyPressed) {
      lanternsOn = !lanternsOn; // Toggle
      lKeyPressed = true;
    }
  } else {
    lKeyPressed = false;
  }

  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
    if (!tKeyPressed) {
      texturesEnabled = !texturesEnabled; // Toggle
      tKeyPressed = true;
    }
  } else {
    tKeyPressed = false;
  }

  if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
    if (!mKeyPressed) {
      audioEnabled = !audioEnabled;
      if (audioEnabled) {
        if (!sideDoorZoneMuted || sideDoorUnlocked) {
          if (horrorTrackStarted) {
            startBackgroundMusic("resources/horror_sound.mp3");
          } else {
            startBackgroundMusic("resources/arabian_nights.mp3");
          }
        }
      } else {
        stopBackgroundMusic();
      }
      mKeyPressed = true;
    }
  } else {
    mKeyPressed = false;
  }

  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    // Reset camera position and orientation
    if (inExterior) {
      camera.Position = glm::vec3(0.0f, 2.0f, 75.0f);
    } else {
      camera.Position = glm::vec3(0.0f, CAMERA_EYE_HEIGHT, -5.0f);
    }
    camera.Yaw = -90.0f;
    camera.Pitch = 0.0f;
    camera.updateCameraVectors();
  }

  // Dev shortcut: jump directly into the unlocked second room.
  if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
    if (!jKeyPressed) {
      inExterior = false;
      sideDoorUnlocked = true;
      sideDoorOpenAmount = 1.0f;
      sideDoorZoneMuted = false;
      camera.Position = glm::vec3(-12.0f, CAMERA_EYE_HEIGHT, -46.6f);
      camera.Yaw = -90.0f;
      camera.Pitch = 0.0f;
      camera.updateCameraVectors();
      jKeyPressed = true;
    }
  } else {
    jKeyPressed = false;
  }

  // Dev shortcut: jump to shoreline facing the water body.
  if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
    if (!kKeyPressed) {
      inExterior = true;
      camera.Position = glm::vec3(0.0f, CAMERA_EYE_HEIGHT, -235.0f);
      camera.Yaw = -90.0f;
      camera.Pitch = -5.0f;
      camera.updateCameraVectors();
      kKeyPressed = true;
    }
  } else {
    kKeyPressed = false;
  }

  // Dev shortcut: teleport to top of main pyramid.
  if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
    if (!pKeyPressed) {
      inExterior = true;
      // Main pyramid: center (0, -15), 45 steps × 1.0 height = 45 units tall
      camera.Position = glm::vec3(0.0f, 47.0f, -15.0f);
      camera.Yaw = -90.0f;
      camera.Pitch = -15.0f;
      camera.updateCameraVectors();
      pKeyPressed = true;
    }
  } else {
    pKeyPressed = false;
  }

  // Dev shortcut: teleport into the first room (interior) directly.
  if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
    if (!iKeyPressed) {
      inExterior = false;
      camera.Position = glm::vec3(0.0f, 1.5f, -5.0f);
      camera.Yaw = -90.0f;
      camera.Pitch = 0.0f;
      camera.updateCameraVectors();
      iKeyPressed = true;
    }
  } else {
    iKeyPressed = false;
  }

  // Dev shortcut: teleport from interior to exterior (outside main door)
  if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
    if (!oKeyPressed) {
      inExterior = true;
      camera.Position = glm::vec3(0.0f, CAMERA_EYE_HEIGHT, 60.0f);
      camera.Yaw = -90.0f; // Look towards the pyramid
      camera.Pitch = 0.0f;
      camera.updateCameraVectors();
      oKeyPressed = true;
    }
  } else {
    oKeyPressed = false;
  }

  // Dev shortcut: teleport directly in front of a fractal tree
  static bool uKeyPressed = false;
  if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
    if (!uKeyPressed) {
      inExterior = true;
      camera.Position = glm::vec3(-185.0f, CAMERA_EYE_HEIGHT - 0.5f, 60.0f);
      camera.Yaw = 180.0f; // Look West towards the tree at x=-200 
      camera.Pitch = 15.0f; // Look slightly up at the canopy
      camera.updateCameraVectors();
      uKeyPressed = true;
    }
  } else {
    uKeyPressed = false;
  }

  // Dev shortcut: toggle swinging trap (H for Halt)
  if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
    if (!hKeyPressed) {
      bladeActive = !bladeActive;
      hKeyPressed = true;
    }
  } else {
    hKeyPressed = false;
  }

  // Dev shortcut: toggle 4-viewport mode (V)
  if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
    if (!vKeyPressed) {
      multiViewportMode = !multiViewportMode;
      vKeyPressed = true;
    }
  } else {
    vKeyPressed = false;
  }

  // ========== TWEAKABLE GRAPHICS CONCEPT SHORTCUTS ==========

  // --- Category 1: Geometry & Modeling ---
  // [ / ]: Bezier wave amplitude (water surface)
  if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) {
    bezierAmplitude += 2.0f * deltaTime;
    if (bezierAmplitude > 5.0f) bezierAmplitude = 5.0f;
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) {
    bezierAmplitude -= 2.0f * deltaTime;
    if (bezierAmplitude < 0.0f) bezierAmplitude = 0.0f;
  }

  // , / . : Fractal tree depth (frond density)
  static bool fracUpPressed = false;
  static bool fracDownPressed = false;
  if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) {
    if (!fracUpPressed) {
      fractalDepth = glm::min(fractalDepth + 1, 6);
      fracUpPressed = true;
    }
  } else { fracUpPressed = false; }
  if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) {
    if (!fracDownPressed) {
      fractalDepth = glm::max(fractalDepth - 1, 1);
      fracDownPressed = true;
    }
  } else { fracDownPressed = false; }

  // --- Category 3: Lighting Sources ---
  // 1: Toggle Directional Light (Sun)
  static bool key1Pressed = false;
  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
    if (!key1Pressed) { directionalLightOn = !directionalLightOn; key1Pressed = true; }
  } else { key1Pressed = false; }

  // 2: Toggle Point Lights (Lanterns)
  static bool key2Pressed = false;
  if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
    if (!key2Pressed) { pointLightsOn = !pointLightsOn; key2Pressed = true; }
  } else { key2Pressed = false; }

  // 3: Toggle Spot Light (Flashlight) — already F, but 3 adds a second shortcut
  static bool key3Pressed = false;
  if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
    if (!key3Pressed) { flashlightOn = !flashlightOn; key3Pressed = true; }
  } else { key3Pressed = false; }

  // --- Category 4: Shading & Illumination ---
  // 4: Toggle Ambient component
  static bool key4Pressed = false;
  if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
    if (!key4Pressed) { ambientOn = !ambientOn; key4Pressed = true; }
  } else { key4Pressed = false; }

  // 5: Toggle Diffuse component
  static bool key5Pressed = false;
  if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
    if (!key5Pressed) { diffuseOn = !diffuseOn; key5Pressed = true; }
  } else { key5Pressed = false; }

  // 6: Toggle Specular component
  static bool key6Pressed = false;
  if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
    if (!key6Pressed) { specularOn = !specularOn; key6Pressed = true; }
  } else { key6Pressed = false; }

  // 7: Toggle Gouraud vs Phong shading
  static bool key7Pressed = false;
  if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
    if (!key7Pressed) { useGouraud = !useGouraud; key7Pressed = true; }
  } else { key7Pressed = false; }

  // 8 / 9: Adjust Specular Power (Phong shininess exponent)
  if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
    specularPower -= 30.0f * deltaTime;
    if (specularPower < 2.0f) specularPower = 2.0f;
  }
  if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
    specularPower += 30.0f * deltaTime;
    if (specularPower > 256.0f) specularPower = 256.0f;
  }

  // 0: Adjust Spotlight cone angle
  static bool key0Pressed = false;
  if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
    if (!key0Pressed) {
      // Cycle through cone angles: 14 -> 25 -> 40 -> 60 -> 14
      if (spotConeAngle < 20.0f) spotConeAngle = 25.0f;
      else if (spotConeAngle < 30.0f) spotConeAngle = 40.0f;
      else if (spotConeAngle < 50.0f) spotConeAngle = 60.0f;
      else spotConeAngle = 14.0f;
      key0Pressed = true;
    }
  } else { key0Pressed = false; }

  // Interaction
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    if (!eKeyPressed) {
      if (inExterior) {
        float dist = glm::length(camera.Position - glm::vec3(0.0f, CAMERA_EYE_HEIGHT, 33.0f));
        if (dist < 5.0f) {
           inExterior = false;
           camera.Position = glm::vec3(0.0f, 1.5f, -5.0f);
           camera.Yaw = -90.0f;
           camera.Pitch = 0.0f;
           camera.updateCameraVectors();
        }
      } else {
        float dist = glm::length(camera.Position - glm::vec3(0.0f, 0.0f, -20.0f));
        if (dist < 5.0f) {
          sarcophagusInteract = true;
          sarcophagusOpen = !sarcophagusOpen; // Toggle
        }
      }
      eKeyPressed = true;
    }
  } else {
      eKeyPressed = false;
  }

  constrainCameraToTomb();
}

void constrainCameraToTomb() {
  if (inExterior) {
    camera.Position.x = glm::clamp(camera.Position.x, -800.0f, 800.0f);
    camera.Position.z = glm::clamp(camera.Position.z, -800.0f, 800.0f);
    // Allow vertical movement but keep above ground
    camera.Position.y = glm::max(camera.Position.y, CAMERA_EYE_HEIGHT);
    return;
  }

  // Zone-based collision for interior spaces so walls remain solid:
  // 1) Main hall
  // 2) Doorway throat
  // 3) Second chamber
  const float hallMinX = -4.6f;
  const float hallMaxX = CAMERA_MAX_X;
  const float hallMinZ = CAMERA_MIN_Z;
  const float hallMaxZ = CAMERA_MAX_Z;

  const float chamberMinX = CAMERA_MIN_X;
  const float chamberMaxX = -5.4f; // Keep player inside chamber side of shared wall.
  const float chamberMinZ = -52.0f;
  const float chamberMaxZ = -42.8f;

  const float doorwayMinZ = -48.6f;
  const float doorwayMaxZ = -46.4f;

  float x = camera.Position.x;
  float z = camera.Position.z;

  // Door not open yet: keep player in main hall only.
  if (!sideDoorUnlocked || sideDoorOpenAmount < 0.45f) {
    camera.Position.x = glm::clamp(x, hallMinX, hallMaxX);
    camera.Position.z = glm::clamp(z, hallMinZ, hallMaxZ);
    camera.Position.y = CAMERA_EYE_HEIGHT;
    return;
  }

  bool inDoorwayZ = (z >= doorwayMinZ && z <= doorwayMaxZ);
  bool chamberSide = (x <= -5.0f);

  if (inDoorwayZ) {
    // Through the opening, player can move between hall and chamber.
    x = glm::clamp(x, chamberMinX, hallMaxX);
    z = glm::clamp(z, hallMinZ, hallMaxZ);
  } else if (chamberSide) {
    x = glm::clamp(x, chamberMinX, chamberMaxX);
    z = glm::clamp(z, chamberMinZ, chamberMaxZ);
  } else {
    x = glm::clamp(x, hallMinX, hallMaxX);
    z = glm::clamp(z, hallMinZ, hallMaxZ);
  }

  camera.Position.x = x;
  camera.Position.z = z;
  camera.Position.y = CAMERA_EYE_HEIGHT;
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

void char_callback(GLFWwindow *window, unsigned int codepoint) {
  if (!sideDoorZoneMuted || sideDoorUnlocked)
    return;

  if (codepoint >= 'a' && codepoint <= 'z')
    codepoint = codepoint - 'a' + 'A';

  if (codepoint >= 'A' && codepoint <= 'Z') {
    sideDoorInputBuffer.push_back(static_cast<char>(codepoint));
    std::cout << "Door input: " << sideDoorInputBuffer << std::endl;
  }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (!sideDoorZoneMuted || sideDoorUnlocked)
    return;

  if (action != GLFW_PRESS && action != GLFW_REPEAT)
    return;

  if (key == GLFW_KEY_BACKSPACE) {
    if (!sideDoorInputBuffer.empty()) {
      sideDoorInputBuffer.pop_back();
      std::cout << "Door input: " << sideDoorInputBuffer << std::endl;
    }
    return;
  }

  if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) {
    if (sideDoorInputBuffer == SIDE_DOOR_CODE) {
      sideDoorUnlocked = true;
      stopBackgroundMusic();
      if (!horrorTrackStarted)
        horrorTrackStarted = true;
      if (audioEnabled)
        startBackgroundMusic("resources/horror_sound.mp3");
      sideDoorZoneMuted = false;
      std::cout << "The seal breaks. The stone doors open." << std::endl;
    } else {
      std::cout << "Wrong code. The seal remains." << std::endl;
    }
    sideDoorInputBuffer.clear();
  }
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
                     float slideAmount, unsigned int textureID, bool useTexture) {
  // Common texture setup
  shader.setBool("useTexture", useTexture);
  glBindTexture(GL_TEXTURE_2D, textureID);
  shader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));
  shader.setVec2("uvOffset", glm::vec2(0.0f, 0.0f));

  // Base
  glm::mat4 base = glm::scale(parentModel, glm::vec3(1.5f, 1.0f, 3.0f));
  shader.setMat4("model", base);
  shader.setVec3("objectColor", 1.0f, 0.9f, 0.8f); // Bright base for texture
  cube.draw(shader.ID);

  // Lid (Sliding)
  glm::mat4 lid = glm::translate(
      parentModel, glm::vec3(0.0f, 0.6f, slideAmount)); // Slide along Z
  lid = glm::scale(lid, glm::vec3(1.6f, 0.2f, 3.1f));
  shader.setMat4("model", lid);
  shader.setVec3("objectColor", 1.0f, 1.0f, 1.0f); // Bright for lid detail
  cube.draw(shader.ID);
}

void drawSecondRoomBurialSet(Shader &shader, Cube &cube, Cylinder &cyl,
                             glm::vec3 center, unsigned int stoneTexture,
                             unsigned int ornamentTexture, bool useTexture) {
  shader.setBool("rotateUV90", false);
  shader.setVec2("uvOffset", glm::vec2(0.0f, 0.0f));

  // Layered stone dais
  shader.setBool("useTexture", useTexture);
  glBindTexture(GL_TEXTURE_2D, stoneTexture);
  shader.setVec3("objectColor", 0.62f, 0.56f, 0.48f);

  glm::mat4 step1 = glm::mat4(1.0f);
  step1 = glm::translate(step1, center + glm::vec3(0.0f, 0.18f, 0.0f));
  step1 = glm::scale(step1, glm::vec3(6.6f, 0.36f, 5.6f));
  shader.setMat4("model", step1);
  shader.setVec2("uvScale", glm::vec2(4.0f, 3.0f));
  cube.draw(shader.ID);

  glm::mat4 step2 = glm::mat4(1.0f);
  step2 = glm::translate(step2, center + glm::vec3(0.0f, 0.45f, 0.0f));
  step2 = glm::scale(step2, glm::vec3(5.1f, 0.20f, 4.2f));
  shader.setMat4("model", step2);
  shader.setVec2("uvScale", glm::vec2(3.0f, 2.4f));
  cube.draw(shader.ID);

  glm::mat4 step3 = glm::mat4(1.0f);
  step3 = glm::translate(step3, center + glm::vec3(0.0f, 0.63f, 0.0f));
  step3 = glm::scale(step3, glm::vec3(3.7f, 0.16f, 3.0f));
  shader.setMat4("model", step3);
  shader.setVec2("uvScale", glm::vec2(2.4f, 2.0f));
  cube.draw(shader.ID);

  // Ornate coffin/chest
  shader.setBool("useTexture", useTexture);
  glBindTexture(GL_TEXTURE_2D, ornamentTexture);
  shader.setVec3("objectColor", 0.96f, 0.84f, 0.62f);

  glm::mat4 coffinBase = glm::mat4(1.0f);
  coffinBase = glm::translate(coffinBase, center + glm::vec3(-0.10f, 1.00f, -0.02f));
  coffinBase = glm::scale(coffinBase, glm::vec3(2.15f, 0.62f, 1.16f));
  shader.setMat4("model", coffinBase);
  shader.setVec2("uvScale", glm::vec2(2.2f, 1.2f));
  cube.draw(shader.ID);

  glm::mat4 coffinLid = glm::mat4(1.0f);
  coffinLid = glm::translate(coffinLid, center + glm::vec3(-0.10f, 1.38f, -0.02f));
  coffinLid = glm::scale(coffinLid, glm::vec3(2.30f, 0.12f, 1.28f));
  shader.setMat4("model", coffinLid);
  shader.setVec2("uvScale", glm::vec2(2.3f, 1.0f));
  cube.draw(shader.ID);

  // Gold trim strips on coffin
  shader.setBool("useTexture", false);
  shader.setVec3("objectColor", 0.82f, 0.62f, 0.28f);

  glm::mat4 trim1 = glm::mat4(1.0f);
  trim1 = glm::translate(trim1, center + glm::vec3(-0.10f, 1.16f, 0.57f));
  trim1 = glm::scale(trim1, glm::vec3(1.92f, 0.05f, 0.06f));
  shader.setMat4("model", trim1);
  cube.draw(shader.ID);

  glm::mat4 trim2 = glm::mat4(1.0f);
  trim2 = glm::translate(trim2, center + glm::vec3(-0.10f, 1.16f, -0.57f));
  trim2 = glm::scale(trim2, glm::vec3(1.92f, 0.05f, 0.06f));
  shader.setMat4("model", trim2);
  cube.draw(shader.ID);

  shader.setBool("useTexture", useTexture);
  shader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));
}

void drawLantern(Shader &shader, Cube &cube, Cylinder &cyl, glm::mat4 model,
                 float time, unsigned int textureID, bool useTexture) {
  // 1. Wall bracket — extends straight out from wall
  shader.setBool("useEmissive", false); // Must be false to see texture!
  shader.setBool("useTexture", useTexture);
  shader.setVec2("uvScale", glm::vec2(1.0f, 1.0f)); // Reset scale
  shader.setVec2("uvOffset", glm::vec2(0.0f, 0.0f));
  glBindTexture(GL_TEXTURE_2D, textureID);
  shader.setVec3("objectColor", 1.0f, 1.0f,
                 1.0f); // Bright base for dark texture

  // Horizontal arm
  glm::mat4 bracket = glm::translate(model, glm::vec3(0.2f, 0.0f, 0.0f));
  bracket = glm::scale(bracket, glm::vec3(0.4f, 0.06f, 0.06f));
  shader.setMat4("model", bracket);
  cube.draw(shader.ID);

  // 2. Torch handle — vertical, at end of bracket
  glm::mat4 torchBase = glm::translate(model, glm::vec3(0.4f, 0.0f, 0.0f));

  glm::mat4 handleGeom =
      glm::translate(torchBase, glm::vec3(0.0f, 0.15f, 0.0f));
  handleGeom = glm::scale(handleGeom, glm::vec3(0.05f, 0.5f, 0.05f));
  shader.setMat4("model", handleGeom);
  cyl.draw(shader.ID);

  // 3. Metal cup at top — holds the fire
  glm::mat4 cup = glm::translate(torchBase, glm::vec3(0.0f, 0.4f, 0.0f));

  glm::mat4 cupGeom = glm::scale(cup, glm::vec3(0.1f, 0.08f, 0.1f));
  shader.setMat4("model", cupGeom);
  cyl.draw(shader.ID);

  // 4. FIRE — additive blending
  if (time > 0.0f) { // Only draw fire if time is advancing (lanterns are ON)
    shader.setBool("useTexture", false);
    shader.setBool("useEmissive", true);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending
    glDepthMask(GL_FALSE);             // Don't write depth for transparent fire

    float fl1 = 0.82f + 0.18f * sin(time * 9.0f);
    float fl2 = 0.85f + 0.15f * cos(time * 13.0f + 1.1f);
    float fl3 = 0.78f + 0.22f * sin(time * 17.0f + 2.5f);
    float swayX = 0.02f * sin(time * 5.0f);
    float swayZ = 0.012f * cos(time * 7.0f);

    // Base glow (wide, deep red-orange)
    shader.setVec3("emissiveColor", 0.6f, 0.15f, 0.02f);
    glm::mat4 fb =
        glm::translate(cup, glm::vec3(swayX * 0.3f, 0.08f, swayZ * 0.3f));
    fb = glm::scale(fb, glm::vec3(0.09f * fl1, 0.07f, 0.09f * fl1));
    shader.setMat4("model", fb);
    cyl.draw(shader.ID);

    // Lower flame (orange)
    shader.setVec3("emissiveColor", 1.0f, 0.35f, 0.04f);
    glm::mat4 f1 =
        glm::translate(cup, glm::vec3(swayX * 0.6f, 0.14f, swayZ * 0.5f));
    f1 = glm::scale(f1, glm::vec3(0.065f * fl2, 0.10f * fl1, 0.065f * fl2));
    shader.setMat4("model", f1);
    cyl.draw(shader.ID);

    // Mid flame (bright orange)
    shader.setVec3("emissiveColor", 1.0f, 0.55f, 0.08f);
    glm::mat4 f2 = glm::translate(cup, glm::vec3(swayX, 0.22f, swayZ * 0.8f));
    f2 = glm::scale(f2, glm::vec3(0.045f * fl3, 0.12f * fl2, 0.045f * fl3));
    shader.setMat4("model", f2);
    cyl.draw(shader.ID);

    // Upper flame (yellow, narrowing)
    shader.setVec3("emissiveColor", 1.0f, 0.75f, 0.15f);
    glm::mat4 f3 = glm::translate(cup, glm::vec3(swayX * 1.5f, 0.32f, swayZ));
    f3 = glm::scale(f3, glm::vec3(0.028f * fl1, 0.10f * fl3, 0.028f * fl1));
    shader.setMat4("model", f3);
    cyl.draw(shader.ID);

    // Flame tip (bright yellow-white wisp)
    shader.setVec3("emissiveColor", 1.0f, 0.9f, 0.45f);
    glm::mat4 f4 =
        glm::translate(cup, glm::vec3(swayX * 2.0f, 0.40f, swayZ * 1.5f));
    f4 = glm::scale(f4, glm::vec3(0.012f, 0.08f * fl2, 0.012f));
    shader.setMat4("model", f4);
    cyl.draw(shader.ID);

    // Restore normal blending
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader.setBool("useEmissive", false);
  }
}

float pseudoNoise01(float value) {
  float s = sin(value * 91.37f + 37.11f) * 43758.5453f;
  return s - floor(s);
}

void drawDatePalmFrond(Shader &shader, Cube &cube, glm::mat4 crownModel,
                       float yawDeg, float tiltDeg, float length, float width,
                       float curveDeg, float dryFactor, unsigned int canopyTexture,
                       bool useTexture) {
  bool canopyTextured = useTexture && canopyTexture != 0;
  shader.setBool("useTexture", canopyTextured);
  if (canopyTextured) {
    glBindTexture(GL_TEXTURE_2D, canopyTexture);
    shader.setVec2("uvScale", glm::vec2(0.65f, 2.6f));
  }

  glm::vec3 frondGreen(0.34f, 0.42f, 0.22f);
  glm::vec3 frondDry(0.58f, 0.50f, 0.33f);
  shader.setVec3("objectColor", glm::mix(frondGreen, frondDry, dryFactor));

  glm::mat4 frame = glm::rotate(crownModel, glm::radians(yawDeg),
                                glm::vec3(0.0f, 1.0f, 0.0f));
  frame = glm::rotate(frame, glm::radians(-tiltDeg), glm::vec3(1.0f, 0.0f, 0.0f));

  const int segments = 9;
  for (int s = 0; s < segments; s++) {
    float t = (float)s / (float)(segments - 1);
    float segLen = length / (float)segments;
    float segWidth = width * (1.0f - t * 0.72f);
    float segBend = curveDeg * (0.25f + 0.75f * t);
    float leafletLen = segLen * (0.62f - t * 0.20f);
    float leafletWidth = segWidth * (0.55f - t * 0.15f);

    frame = glm::rotate(frame, glm::radians(segBend), glm::vec3(1.0f, 0.0f, 0.0f));

    // Central frond rib
    glm::mat4 cardA = glm::translate(frame, glm::vec3(0.0f, 0.0f, segLen * 0.5f));
    cardA = glm::scale(cardA, glm::vec3(segWidth, 0.010f, segLen));
    shader.setMat4("model", cardA);
    cube.draw(shader.ID);

    // Leaflets on both sides of the rib (feathered date palm shape)
    for (int side = -1; side <= 1; side += 2) {
      float sideSign = (float)side;

      glm::mat4 leaflet = glm::translate(
          frame, glm::vec3(sideSign * (segWidth * 0.42f), 0.0f, segLen * 0.5f));
      leaflet = glm::rotate(leaflet, glm::radians(sideSign * (52.0f - t * 18.0f)),
                            glm::vec3(0.0f, 0.0f, 1.0f));
      leaflet = glm::rotate(leaflet, glm::radians(-6.0f + t * 10.0f),
                            glm::vec3(1.0f, 0.0f, 0.0f));
      leaflet = glm::scale(leaflet,
                           glm::vec3(leafletWidth, 0.009f, leafletLen));
      shader.setMat4("model", leaflet);
      cube.draw(shader.ID);

      // Cross card helps preserve frond thickness from multiple view angles.
      glm::mat4 leafletCross = glm::rotate(leaflet, glm::radians(84.0f),
                                           glm::vec3(0.0f, 1.0f, 0.0f));
      shader.setMat4("model", leafletCross);
      cube.draw(shader.ID);
    }

    frame = glm::translate(frame, glm::vec3(0.0f, 0.0f, segLen));
  }

  shader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));
}

void drawFractalDesertTree(Shader &shader, Cylinder &cyl, Cube &cube,
                           glm::vec3 position, float scale, float yawDeg,
                           float seed, unsigned int barkTexture,
                           unsigned int canopyTexture, bool useTexture) {
  glm::mat4 root = glm::mat4(1.0f);
  root = glm::translate(root, position);
  root = glm::rotate(root, glm::radians(yawDeg), glm::vec3(0.0f, 1.0f, 0.0f));
  root = glm::scale(root, glm::vec3(scale));

  // Date palms are mostly straight, with subtle wind lean.
  float leanX = (pseudoNoise01(seed * 7.3f) - 0.5f) * 4.0f;
  float leanZ = (pseudoNoise01(seed * 13.1f) - 0.5f) * 4.0f;
  root = glm::rotate(root, glm::radians(leanX), glm::vec3(1.0f, 0.0f, 0.0f));
  root = glm::rotate(root, glm::radians(leanZ), glm::vec3(0.0f, 0.0f, 1.0f));

  float trunkLength = 2.8f + pseudoNoise01(seed * 3.7f) * 0.6f;
  float trunkRadius = 0.15f + pseudoNoise01(seed * 5.1f) * 0.03f;

  bool barkTextured = useTexture && barkTexture != 0;
  shader.setBool("useTexture", barkTextured);
  if (barkTextured) {
    glBindTexture(GL_TEXTURE_2D, barkTexture);
    shader.setVec2("uvScale", glm::vec2(1.0f, trunkLength * 2.8f));
  }
  shader.setVec3("objectColor", 0.72f, 0.60f, 0.44f);

  // Build a tapered trunk from stacked cylinder segments.
  const int trunkSegments = 11;
  const float trunkBaseScale = 1.50f;
  const float trunkTopScale = 0.78f;
  float segLen = trunkLength / (float)trunkSegments;
  for (int s = 0; s < trunkSegments; s++) {
    float t = (float)s / (float)(trunkSegments - 1);  // 0 at base, 1 at top
    float segRadius = trunkRadius * glm::mix(trunkBaseScale, trunkTopScale, t);
    float y = segLen * (0.5f + (float)s);

    if (barkTextured) {
      shader.setVec2("uvScale", glm::vec2(1.0f, 2.0f + trunkLength * (1.2f - 0.6f * t)));
    }

    glm::mat4 trunkSeg = glm::translate(root, glm::vec3(0.0f, y, 0.0f));
    trunkSeg = glm::scale(trunkSeg, glm::vec3(segRadius, segLen, segRadius));
    shader.setMat4("model", trunkSeg);
    cyl.draw(shader.ID);
  }

  // Horizontal scar rings emulate old frond cuts on date palm trunks.
  shader.setBool("useTexture", false);
  for (int i = 0; i < 11; i++) {
    float t = (float)i / 10.0f;
    float y = trunkLength * (0.12f + t * 0.75f);
    float ringRadius = trunkRadius *
                       glm::mix(trunkBaseScale, trunkTopScale, t) * 1.06f;
    glm::mat4 ring = glm::translate(root, glm::vec3(0.0f, y, 0.0f));
    ring = glm::scale(ring, glm::vec3(ringRadius, 0.012f, ringRadius));
    shader.setMat4("model", ring);
    shader.setVec3("objectColor", 0.53f, 0.43f, 0.30f);
    cyl.draw(shader.ID);
  }

  glm::mat4 crown = glm::translate(root, glm::vec3(0.0f, trunkLength, 0.0f));

  shader.setBool("useTexture", false);
  shader.setVec3("objectColor", 0.45f, 0.34f, 0.23f);
  glm::mat4 crownCore =
      glm::scale(crown, glm::vec3(trunkRadius * 1.18f, 0.16f, trunkRadius * 1.18f));
  shader.setMat4("model", crownCore);
  cyl.draw(shader.ID);

  int frondCount = fractalDepth * 6; // 6 to 36 fronds based on fractalDepth (1-6)
  for (int i = 0; i < frondCount; i++) {
    float n0 = pseudoNoise01(seed * 9.1f + i * 0.77f);
    float n1 = pseudoNoise01(seed * 11.3f + i * 1.21f);
    float n2 = pseudoNoise01(seed * 15.7f + i * 1.63f);

    float yaw = (360.0f / (float)frondCount) * i + (n0 - 0.5f) * 24.0f;
    float tilt = 36.0f + n1 * 20.0f;
    float length = 1.08f + n2 * 0.56f;
    float width = 0.14f + n0 * 0.05f;
    float curve = 5.0f + n1 * 5.0f;
    float dryFactor = 0.45f + n2 * 0.35f;

    // A few older fronds hang lower and drier.
    if (i % 6 == 0) {
      tilt += 15.0f;
      dryFactor = glm::min(1.0f, dryFactor + 0.25f);
      length *= 0.88f;
    }

    drawDatePalmFrond(shader, cube, crown, yaw, tilt, length, width, curve,
                      dryFactor, canopyTexture, useTexture);
  }

  // Inner crown layer: shorter, more upright fronds for dense date-palm top.
  int innerFrondCount = fractalDepth * 3; // 3 to 18 inner fronds
  for (int i = 0; i < innerFrondCount; i++) {
    float n0 = pseudoNoise01(seed * 19.1f + i * 0.91f);
    float n1 = pseudoNoise01(seed * 23.3f + i * 1.37f);
    float n2 = pseudoNoise01(seed * 29.7f + i * 1.71f);

    float yaw = (360.0f / (float)innerFrondCount) * i + (n0 - 0.5f) * 16.0f;
    float tilt = 18.0f + n1 * 14.0f;
    float length = 0.78f + n2 * 0.34f;
    float width = 0.12f + n0 * 0.04f;
    float curve = 2.0f + n1 * 3.0f;
    float dryFactor = 0.28f + n2 * 0.22f;

    drawDatePalmFrond(shader, cube, crown, yaw, tilt, length, width, curve,
                      dryFactor, canopyTexture, useTexture);
  }
}

// ============================================================
// CAMEL — Hierarchical procedural model with walking animation
// ============================================================
void drawCamel(Shader &shader, Cube &cube, Cylinder &cyl, glm::mat4 rootModel,
               float walkPhase, unsigned int textureID, bool useTexture) {
  shader.setBool("useEmissive", false);
  shader.setBool("useTexture", useTexture);
  shader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));
  shader.setVec2("uvOffset", glm::vec2(0.0f, 0.0f));
  shader.setBool("rotateUV90", false);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);

  // Camel color tint (warm sandy brown — multiplied with texture)
  glm::vec3 bodyColor(0.85f, 0.72f, 0.55f);
  glm::vec3 bellyColor(0.78f, 0.68f, 0.52f);
  glm::vec3 legColor(0.75f, 0.62f, 0.48f);
  glm::vec3 headColor(0.82f, 0.70f, 0.54f);
  glm::vec3 darkColor(0.45f, 0.35f, 0.25f);  // hooves, eyes, nose

  // Walking animation parameters — diagonal gait
  float legSwing = 0.35f;  // max swing angle in radians
  float bodyBob = 0.03f * sin(walkPhase * 2.0f);  // subtle body movement

  // Leg phases: diagonal gait (FL syncs with BR, FR syncs with BL)
  float legFL = sin(walkPhase);       // Front-Left
  float legFR = sin(walkPhase + 3.14159f);  // Front-Right (opposite)
  float legBL = sin(walkPhase + 3.14159f);  // Back-Left (same as FR)
  float legBR = sin(walkPhase);       // Back-Right (same as FL)

  // Body center at proper height above ground
  float bodyHeight = 2.2f + bodyBob;
  glm::mat4 bodyBase = glm::translate(rootModel, glm::vec3(0.0f, bodyHeight, 0.0f));

  // === TORSO (main body) ===
  glm::mat4 torso = glm::scale(bodyBase, glm::vec3(1.2f, 0.9f, 2.4f));
  shader.setMat4("model", torso);
  shader.setVec3("objectColor", bodyColor);
  shader.setVec2("uvScale", glm::vec2(2.0f, 1.0f));
  cube.draw(shader.ID);

  // === BELLY (slightly wider underbody for realism) ===
  glm::mat4 belly = glm::translate(bodyBase, glm::vec3(0.0f, -0.35f, 0.0f));
  belly = glm::scale(belly, glm::vec3(1.0f, 0.25f, 2.0f));
  shader.setMat4("model", belly);
  shader.setVec3("objectColor", bellyColor);
  shader.setVec2("uvScale", glm::vec2(1.5f, 1.0f));
  cube.draw(shader.ID);

  // === HUMP (single dromedary hump) ===
  glm::mat4 hump = glm::translate(bodyBase, glm::vec3(0.0f, 0.65f, -0.15f));
  hump = glm::scale(hump, glm::vec3(0.7f, 0.55f, 0.9f));
  shader.setMat4("model", hump);
  shader.setVec3("objectColor", bodyColor * 0.95f);
  shader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));
  cube.draw(shader.ID);

  // Hump peak (rounded top)
  glm::mat4 humpTop = glm::translate(bodyBase, glm::vec3(0.0f, 0.88f, -0.15f));
  humpTop = glm::scale(humpTop, glm::vec3(0.45f, 0.2f, 0.55f));
  shader.setMat4("model", humpTop);
  cube.draw(shader.ID);

  // === NECK (angled forward, multi-segment for smooth curve) ===
  // Lower neck
  glm::mat4 neckBase = glm::translate(bodyBase, glm::vec3(0.0f, 0.25f, 1.1f));
  neckBase = glm::rotate(neckBase, glm::radians(35.0f), glm::vec3(1.0f, 0.0f, 0.0f));

  glm::mat4 neckLower = glm::translate(neckBase, glm::vec3(0.0f, 0.45f, 0.0f));
  neckLower = glm::scale(neckLower, glm::vec3(0.45f, 0.9f, 0.4f));
  shader.setMat4("model", neckLower);
  shader.setVec3("objectColor", bodyColor);
  shader.setVec2("uvScale", glm::vec2(0.5f, 1.5f));
  cube.draw(shader.ID);

  // Upper neck (slightly thinner, more vertical)
  glm::mat4 neckUpper = glm::translate(neckBase, glm::vec3(0.0f, 1.1f, 0.0f));
  neckUpper = glm::rotate(neckUpper, glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  
  glm::mat4 neckUpperGeom = glm::scale(neckUpper, glm::vec3(0.35f, 0.7f, 0.32f));
  shader.setMat4("model", neckUpperGeom);
  shader.setVec3("objectColor", bodyColor * 0.98f);
  cube.draw(shader.ID);

  // === HEAD ===
  glm::mat4 headBase = glm::translate(neckUpper, glm::vec3(0.0f, 0.42f, 0.08f));
  headBase = glm::rotate(headBase, glm::radians(-20.0f), glm::vec3(1.0f, 0.0f, 0.0f));

  // Main head (elongated)
  glm::mat4 headGeom = glm::scale(headBase, glm::vec3(0.35f, 0.32f, 0.55f));
  shader.setMat4("model", headGeom);
  shader.setVec3("objectColor", headColor);
  shader.setVec2("uvScale", glm::vec2(0.5f, 0.8f));
  cube.draw(shader.ID);

  // Snout / Muzzle (protruding forward)
  glm::mat4 snout = glm::translate(headBase, glm::vec3(0.0f, -0.08f, 0.28f));
  snout = glm::scale(snout, glm::vec3(0.25f, 0.22f, 0.25f));
  shader.setMat4("model", snout);
  shader.setVec3("objectColor", headColor * 0.9f);
  cube.draw(shader.ID);

  // Nose (dark tip)
  glm::mat4 nose = glm::translate(headBase, glm::vec3(0.0f, -0.06f, 0.42f));
  nose = glm::scale(nose, glm::vec3(0.12f, 0.08f, 0.06f));
  shader.setMat4("model", nose);
  shader.setBool("useTexture", false);
  shader.setVec3("objectColor", darkColor);
  cube.draw(shader.ID);

  // Eyes (dark, on each side)
  for (float side = -1.0f; side <= 1.0f; side += 2.0f) {
    glm::mat4 eye = glm::translate(headBase, glm::vec3(side * 0.16f, 0.05f, 0.18f));
    eye = glm::scale(eye, glm::vec3(0.06f, 0.06f, 0.06f));
    shader.setMat4("model", eye);
    shader.setVec3("objectColor", 0.1f, 0.08f, 0.05f);
    cube.draw(shader.ID);
  }

  // Ears (small, angled)
  for (float side = -1.0f; side <= 1.0f; side += 2.0f) {
    glm::mat4 ear = glm::translate(headBase, glm::vec3(side * 0.15f, 0.2f, -0.08f));
    ear = glm::rotate(ear, glm::radians(side * 15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ear = glm::scale(ear, glm::vec3(0.06f, 0.14f, 0.05f));
    shader.setMat4("model", ear);
    shader.setVec3("objectColor", headColor);
    cube.draw(shader.ID);
  }

  // Restore texture for body
  shader.setBool("useTexture", useTexture);
  glBindTexture(GL_TEXTURE_2D, textureID);

  // === LEGS (4 legs with upper + lower segments, animated) ===
  float legOffsets[4][2] = {
    { 0.4f,  0.85f},  // Front-Left (x, z from body center)
    {-0.4f,  0.85f},  // Front-Right
    { 0.4f, -0.85f},  // Back-Left
    {-0.4f, -0.85f},  // Back-Right
  };
  float legPhases[4] = {legFL, legFR, legBL, legBR};

  for (int i = 0; i < 4; i++) {
    float swing = legPhases[i] * legSwing;
    float kneeAngle = fabs(legPhases[i]) * 0.3f;  // Knee bends more mid-stride

    // Hip joint
    glm::mat4 hip = glm::translate(bodyBase, 
        glm::vec3(legOffsets[i][0], -0.4f, legOffsets[i][1]));
    hip = glm::rotate(hip, swing, glm::vec3(1.0f, 0.0f, 0.0f));

    // Upper leg
    glm::mat4 upperLeg = glm::translate(hip, glm::vec3(0.0f, -0.5f, 0.0f));
    glm::mat4 upperLegGeom = glm::scale(upperLeg, glm::vec3(0.22f, 1.0f, 0.22f));
    shader.setMat4("model", upperLegGeom);
    shader.setVec3("objectColor", legColor);
    shader.setVec2("uvScale", glm::vec2(0.3f, 1.0f));
    cube.draw(shader.ID);

    // Knee joint
    glm::mat4 knee = glm::translate(upperLeg, glm::vec3(0.0f, -0.5f, 0.0f));
    knee = glm::rotate(knee, kneeAngle, glm::vec3(1.0f, 0.0f, 0.0f));

    // Lower leg (slightly thinner)
    glm::mat4 lowerLeg = glm::translate(knee, glm::vec3(0.0f, -0.45f, 0.0f));
    glm::mat4 lowerLegGeom = glm::scale(lowerLeg, glm::vec3(0.17f, 0.9f, 0.17f));
    shader.setMat4("model", lowerLegGeom);
    shader.setVec3("objectColor", legColor * 0.92f);
    cube.draw(shader.ID);

    // Hoof (dark, flat)
    shader.setBool("useTexture", false);
    glm::mat4 hoof = glm::translate(lowerLeg, glm::vec3(0.0f, -0.48f, 0.0f));
    hoof = glm::scale(hoof, glm::vec3(0.19f, 0.08f, 0.22f));
    shader.setMat4("model", hoof);
    shader.setVec3("objectColor", darkColor);
    cube.draw(shader.ID);
    shader.setBool("useTexture", useTexture);
    glBindTexture(GL_TEXTURE_2D, textureID);
  }

  // === TAIL ===
  glm::mat4 tailBase = glm::translate(bodyBase, glm::vec3(0.0f, 0.1f, -1.2f));
  // Slight tail sway
  float tailSway = 0.1f * sin(walkPhase * 0.7f);
  tailBase = glm::rotate(tailBase, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  tailBase = glm::rotate(tailBase, tailSway, glm::vec3(0.0f, 0.0f, 1.0f));

  glm::mat4 tail = glm::translate(tailBase, glm::vec3(0.0f, -0.3f, 0.0f));
  tail = glm::scale(tail, glm::vec3(0.08f, 0.6f, 0.07f));
  shader.setMat4("model", tail);
  shader.setVec3("objectColor", bodyColor * 0.85f);
  shader.setVec2("uvScale", glm::vec2(0.2f, 1.0f));
  cube.draw(shader.ID);

  // Tail tuft (darker, slightly wider)
  glm::mat4 tuft = glm::translate(tailBase, glm::vec3(0.0f, -0.62f, 0.0f));
  tuft = glm::scale(tuft, glm::vec3(0.1f, 0.15f, 0.09f));
  shader.setMat4("model", tuft);
  shader.setVec3("objectColor", darkColor * 1.3f);
  cube.draw(shader.ID);

  // Reset shader state
  shader.setVec2("uvScale", glm::vec2(1.0f, 1.0f));
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
