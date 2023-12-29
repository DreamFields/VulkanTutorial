#include "utils.h"

glm::vec3 RodriguesRotation (glm::vec3 v, float teta, glm::vec3 k)
{
  glm::vec3 v_rot = v * glm::cos(teta) +
    glm::cross(k, v) * glm::sin(teta) +
    k * glm::dot(k, v) * (1.0f - glm::cos(teta));

  return glm::normalize(v_rot);
}

// glm::dvec3 RodriguesRotation (glm::dvec3 v, double teta, glm::dvec3 k)
// {
//   glm::dvec3 v_rot = v * glm::cos(teta) +
//     glm::cross(k, v) * glm::sin(teta) +
//     k * glm::dot(k, v) * (1.0f - glm::cos(teta));

//   return glm::normalize(v_rot);
// }