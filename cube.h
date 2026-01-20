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

#endif
