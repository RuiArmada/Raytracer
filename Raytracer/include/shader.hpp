#pragma once

#include "geometry.hpp"
#include "scene.hpp"
#include <glm/glm.hpp>
#include <tiny_obj_loader.h>

using namespace glm;

class Shader {
protected:
  const Scene &scene;

public:
  explicit Shader(const Scene &scene) : scene{scene} {};

  virtual vec3 getColor(const Intersection &intersection) { return {0, 0, 0}; };
};