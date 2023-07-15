#pragma once

#include "geometry.hpp"
#include "intersection.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <tiny_obj_loader.h>
#include <vector>

// Forward declaration to avoid circular dependency hell :(
class Scene;

using namespace glm;

enum LightType { AMBIENT, POINT, AREA, UNDEFINED };

class Light {
public:
  Light() = default;
  virtual ~Light() = default;

  vec3 rgb{};

  [[nodiscard]] virtual LightType lightType() const;

  /**
   * @brief Environment lighting component, when viewed with direction _ray_.
   * @param ray direction looking at the light
   * @return intensity when viewed from ray
   */
  [[nodiscard]] virtual vec3 Le(vec3 ray) const = 0;
  [[nodiscard]] virtual vec3 sample(const Intersection &intersection,
                                    const Scene &scene) const = 0;

  /**
   * @brief Computes the incident radiation from this light at the intersection
   * @param intersection the intersection where we're sampling the light
   * @param wi output vector, containing the direction to the light source
   * @param scene scene, to check visibility
   * @param pdf probability density function output
   * @return Incident radiation from the light
   */
  [[nodiscard]] virtual vec3 sample_Li(const Intersection &intersection,
                                       vec3 &wi, const Scene &scene,
                                       float &pdf) const = 0;
};

class AmbientLight : public Light {
public:
  vec3 color;

  explicit AmbientLight(vec3 color);

  [[nodiscard]] LightType lightType() const override;
  [[nodiscard]] vec3 Le(vec3 ray) const override { return color; };

  // TODO: This is.. probably... wrong
  [[nodiscard]] vec3 sample(const Intersection &intersection,
                            const Scene &scene) const override;
  [[nodiscard]] vec3 sample_Li(const Intersection &intersection, vec3 &wi, const Scene &scene, float &pdf) const override;
};

class PointLight : public Light {
public:
  vec3 position;
  vec3 color;
  float intensity{1.0f};

  PointLight(vec3 pos, vec3 color, float intensity);
  [[nodiscard]] LightType lightType() const override;
  [[nodiscard]] vec3 Le(vec3 ray) const override { return vec3{0.0f}; };
  [[nodiscard]] vec3 sample(const Intersection &intersection,
                            const Scene &scene) const override;

  [[nodiscard]] vec3 sample_Li(const Intersection &intersection, vec3 &wi,
                               const Scene &scene, float &pdf) const override;
};

class AreaLight : public Light {
public:
  float pdf;
  vec3 position;
  vec3 color;
  vec3 intensity;
  Triangle gem;

  AreaLight(vec3 color, vec3 v1, vec3 v2, vec3 v3);
  [[nodiscard]] LightType lightType() const override;
  [[nodiscard]] vec3 sampleLight(vec2 rand) const;
  [[nodiscard]] vec3 Le(vec3 ray) const override { return vec3{0.0f}; };
  [[nodiscard]] vec3 sample(const Intersection &intersection,
                            const Scene &scene) const override;

  [[nodiscard]] vec3 sample_Li(const Intersection &intersection, vec3 &wi,
                               const Scene &scene, float &pdf) const override { return {};};
};
