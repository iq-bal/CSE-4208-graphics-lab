#ifndef CUBE_H
#define CUBE_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

class Cube {
public:
  unsigned int VAO, VBO, EBO;
  glm::vec3 color;

  Cube(glm::vec3 c = glm::vec3(1.0f, 1.0f, 1.0f)) : color(c) {
    float vertices[] = {// positions          // normals
                        -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  -0.5f,
                        -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  0.5f,  -0.5f, 0.0f,
                        0.0f,  -1.0f, -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,

                        -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f,
                        0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,
                        0.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

                        -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,
                        -0.5f, -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, -1.0f,
                        0.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,

                        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,
                        -0.5f, 1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, -0.5f, 1.0f,
                        0.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,

                        -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f,
                        -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,
                        -1.0f, 0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,

                        -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.5f,  0.5f,
                        -0.5f, 0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,
                        1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f};

    unsigned int indices[] = {0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,
                              8,  9,  10, 10, 11, 8,  12, 13, 14, 14, 15, 12,
                              16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  void draw(unsigned int shaderProgram, glm::mat4 model) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1,
                       GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, &color[0]);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
  }
};

#include <cmath>

class Sphere {
public:
  unsigned int VAO, VBO, EBO;
  glm::vec3 color;
  int indexCount;

  Sphere(glm::vec3 c = glm::vec3(1.0f, 1.0f, 1.0f)) : color(c) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    const unsigned int X_SEGMENTS = 20;
    const unsigned int Y_SEGMENTS = 20;
    const float PI = 3.14159265359f;

    for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
      for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
        float xSegment = (float)x / (float)X_SEGMENTS;
        float ySegment = (float)y / (float)Y_SEGMENTS;
        float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
        float yPos = std::cos(ySegment * PI);
        float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

        vertices.push_back(xPos * 0.5f);
        vertices.push_back(yPos * 0.5f);
        vertices.push_back(zPos * 0.5f);
        vertices.push_back(xPos);
        vertices.push_back(yPos);
        vertices.push_back(zPos);
      }
    }

    for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
      for (unsigned int x = 0; x < X_SEGMENTS; ++x) {
        indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
        indices.push_back(y * (X_SEGMENTS + 1) + x);
        indices.push_back(y * (X_SEGMENTS + 1) + x + 1);

        indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
        indices.push_back(y * (X_SEGMENTS + 1) + x + 1);
        indices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);
      }
    }

    indexCount = indices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0],
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 &indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  void draw(unsigned int shaderProgram, glm::mat4 model) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1,
                       GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, &color[0]);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
  }
};

#endif
