#include "bus_model.h"
#include "material.h"
#include <glm/gtc/matrix_transform.hpp>

BusModel::BusModel() : bodyCube(glm::vec3(1.0f)), wheelSphere(glm::vec3(1.0f)) {
  // Colors are handled by shader uniforms now, but we initialize geometry here
}

void SetMaterial(Shader &shader, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec,
                 float shine, glm::vec3 emissive = glm::vec3(0.0f)) {
  shader.setVec3("material.ambient", amb);
  shader.setVec3("material.diffuse", diff);
  shader.setVec3("material.specular", spec);
  shader.setFloat("material.shininess", shine);
  shader.setVec3("material.emissive", emissive);
}

void BusModel::Draw(Shader &shader, glm::mat4 modelMatrix) {
  DrawBody(shader, modelMatrix);
  DrawInterior(shader, modelMatrix);
  DrawWindows(shader, modelMatrix);
  DrawWheels(shader, modelMatrix);
}

void BusModel::DrawBody(Shader &shader, glm::mat4 model) {
  // Paint Material (White/Grey)
  SetMaterial(shader, glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(0.3f), 32.0f);

  // Floor
  glm::mat4 m = glm::translate(model, glm::vec3(0.0f, -0.7f, 0.0f));
  m = glm::scale(m, glm::vec3(2.0f, 0.1f, 5.0f));
  bodyCube.draw(shader.ID, m);

  // Roof
  m = glm::translate(model, glm::vec3(0.0f, 0.9f, 0.0f)); // Higher roof
  m = glm::scale(m, glm::vec3(2.0f, 0.1f, 5.0f));
  bodyCube.draw(shader.ID, m);

  // Sides (Lower)
  m = glm::translate(model, glm::vec3(-0.95f, -0.35f, 0.0f));
  m = glm::scale(m, glm::vec3(0.1f, 0.6f, 5.0f));
  bodyCube.draw(shader.ID, m);

  m = glm::translate(model, glm::vec3(0.95f, -0.35f, 0.0f));
  m = glm::scale(m, glm::vec3(0.1f, 0.6f, 5.0f));
  bodyCube.draw(shader.ID, m);

  // Front/Back
  m = glm::translate(model, glm::vec3(0.0f, 0.1f, 2.45f));
  m = glm::scale(m, glm::vec3(2.0f, 1.5f, 0.1f));
  bodyCube.draw(shader.ID, m);

  m = glm::translate(model, glm::vec3(0.0f, 0.1f, -2.45f));
  m = glm::scale(m, glm::vec3(2.0f, 1.5f, 0.1f));
  bodyCube.draw(shader.ID, m);
}

void BusModel::DrawInterior(Shader &shader, glm::mat4 model) {
  // 1. Seats (Plastic Blue)
  SetMaterial(shader, glm::vec3(0.1f, 0.1f, 0.3f), glm::vec3(0.2f, 0.2f, 0.8f),
              glm::vec3(0.5f), 32.0f);

  for (int i = 0; i < 4; i++) {
    glm::mat4 s =
        glm::translate(model, glm::vec3(-0.6f, -0.4f, -1.5f + i * 0.9f));
    s = glm::scale(s, glm::vec3(0.5f, 0.1f, 0.5f));
    bodyCube.draw(shader.ID, s);

    // Backrest
    s = glm::translate(model, glm::vec3(-0.6f, -0.1f, -1.7f + i * 0.9f));
    s = glm::scale(s, glm::vec3(0.5f, 0.5f, 0.1f));
    bodyCube.draw(shader.ID, s);

    // Right side
    s = glm::translate(model, glm::vec3(0.6f, -0.4f, -1.5f + i * 0.9f));
    s = glm::scale(s, glm::vec3(0.5f, 0.1f, 0.5f));
    bodyCube.draw(shader.ID, s);

    // Backrest
    s = glm::translate(model, glm::vec3(0.6f, -0.1f, -1.7f + i * 0.9f));
    s = glm::scale(s, glm::vec3(0.5f, 0.5f, 0.1f));
    bodyCube.draw(shader.ID, s);
  }

  // 2. Handrails (Metal Gold)
  SetMaterial(shader, glm::vec3(0.3f, 0.25f, 0.1f), glm::vec3(0.8f, 0.7f, 0.2f),
              glm::vec3(1.0f, 0.9f, 0.5f), 128.0f);

  // Vertical poles
  for (int i = 0; i < 3; i++) {
    glm::mat4 p =
        glm::translate(model, glm::vec3(-0.9f, 0.1f, -1.0f + i * 1.5f));
    p = glm::scale(p, glm::vec3(0.05f, 1.6f, 0.05f));
    bodyCube.draw(shader.ID, p);

    p = glm::translate(model, glm::vec3(0.9f, 0.1f, -1.0f + i * 1.5f));
    p = glm::scale(p, glm::vec3(0.05f, 1.6f, 0.05f));
    bodyCube.draw(shader.ID, p);
  }
  // Horizontal bar
  glm::mat4 h = glm::translate(model, glm::vec3(0.0f, 0.8f, 0.0f));
  h = glm::scale(h, glm::vec3(1.9f, 0.05f, 0.05f));
  bodyCube.draw(shader.ID, h);
}

void BusModel::DrawWindows(Shader &shader, glm::mat4 model) {
  // Glass (Transparent/Blueish) - No real transparency in basic Phong, but
  // speculate high
  SetMaterial(shader, glm::vec3(0.1f, 0.2f, 0.3f), glm::vec3(0.2f, 0.3f, 0.5f),
              glm::vec3(1.0f), 64.0f);

  // Side Windows
  for (int i = 0; i < 3; i++) {
    glm::mat4 w =
        glm::translate(model, glm::vec3(-1.0f, 0.3f, 1.5f - i * 1.5f));
    w = glm::scale(w, glm::vec3(0.02f, 0.6f, 1.1f));
    bodyCube.draw(shader.ID, w);

    if (i != 0) { // Skip door area on right
      w = glm::translate(model, glm::vec3(1.0f, 0.3f, 1.5f - i * 1.5f));
      w = glm::scale(w, glm::vec3(0.02f, 0.6f, 1.1f));
      bodyCube.draw(shader.ID, w);
    }
  }

  // Windshield
  glm::mat4 ws = glm::translate(model, glm::vec3(0.0f, 0.3f, 2.51f));
  ws = glm::scale(ws, glm::vec3(1.8f, 0.8f, 0.02f));
  bodyCube.draw(shader.ID, ws);
}

void BusModel::DrawWheels(Shader &shader, glm::mat4 model) {
  // Rubber (Dark, Low Shininess)
  SetMaterial(shader, glm::vec3(0.02f), glm::vec3(0.1f), glm::vec3(0.1f), 8.0f);

  glm::vec3 wheelPos[] = {
      glm::vec3(-1.1f, -0.75f, 1.5f), glm::vec3(1.1f, -0.75f, 1.5f),
      glm::vec3(-1.1f, -0.75f, -1.5f), glm::vec3(1.1f, -0.75f, -1.5f)};

  for (int i = 0; i < 4; i++) {
    glm::mat4 w = glm::translate(model, wheelPos[i]);
    w = glm::rotate(w, glm::radians(90.0f), glm::vec3(0, 0, 1));
    w = glm::scale(w, glm::vec3(0.6f, 0.2f, 0.6f)); // Cylinder-ish sphere
    wheelSphere.draw(shader.ID, w);
  }
}
