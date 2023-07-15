#pragma once

#include <glm/glm.hpp>

using namespace glm;

struct Ray {
  vec3 origin{};
  vec3 direction{};

  Ray(vec3 origin, vec3 direction) : origin(origin), direction(direction) {}

  /***
   * @brief Adjusts the origin of the ray to be slightly away from the surface
   * @param normal The normal of the surface (must be normalized)
   */
  inline void adjustOrigin(vec3 normal) {
    vec3 offset = 0.0001f * normal;
    // Ensure the ray is away from the surface, even if the normal is pointing
    // the other way
    if (dot(direction, normal) < 0)
      offset = -1.f * offset;
    origin = origin + offset;
  }
};
