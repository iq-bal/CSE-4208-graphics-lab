#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

// Custom implementation of lookAt function
// Returns a view matrix that transforms world coordinates to view coordinates
glm::mat4 customLookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up);

#endif
