#pragma once

#include "shader.hpp"
#include "bsdf.hpp"

class PathTracerShader : public Shader {
private:
  vec3 background{.0f};
  [[nodiscard]] vec3 estimateDirect(const Intersection &intersection, const Light &light, const BSDF &bsdf) const;
  vec3 uniformSampleAllLights(const Intersection &intersection,
                              const BSDF &bsdf) const;

public:
  explicit PathTracerShader(const Scene &scene) : Shader(scene){};
  vec3 getColor(const Intersection &intersection) override;
};
