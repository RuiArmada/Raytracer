#pragma once

#include "glm/vec3.hpp"
#include "intersection.hpp"

using namespace glm;

class BSDF {
private:
  vec3 ns, ss, ts,  geometricNormal;
public:
  const Material *material;

  explicit BSDF(const Intersection &intersection);
  vec3 worldToLocal(vec3 v) const;
  vec3 localToWorld(vec3 v) const;
  vec3 f(vec3 woWorld, vec3 wiWorld) const;
  vec3 sampleF(vec3 woWorld, vec3 &wiWorld, float &pdf) const;
};