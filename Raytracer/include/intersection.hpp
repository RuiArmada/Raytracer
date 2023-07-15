#pragma once

#include <optional>
#include "geometry.hpp"
#include <glm/glm.hpp>

using namespace glm;

struct Intersection {
  vec3 ray;
  vec3 source;
  optional<vec3> pos;
  optional<vec3> shadingNormal;
  optional<vec3> geometricNormal;
  optional<vec3> lightColor;
  const Triangle *face;
};
