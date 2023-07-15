#include "light.hpp"
#include "ray.hpp"
#include "scene.hpp"

LightType Light::lightType() const { return UNDEFINED; }

// Ambient
AmbientLight::AmbientLight(vec3 color) : color(color) {}
LightType AmbientLight::lightType() const { return AMBIENT; }
vec3 AmbientLight::sample_Li(const Intersection &intersection, vec3 &wi,
                             const Scene &scene, float &pdf) const {
  pdf = 1.0f;
  wi = reflect(intersection.ray, *intersection.shadingNormal);
  return color;
}
vec3 AmbientLight::sample(const Intersection &intersection,
                          const Scene &scene) const {
  vec3 sampledColor{.0f};
  auto material = intersection.face->material;

  if (material->baseColor != vec3{.0f}) {
    sampledColor = material->baseColor * color;
  }

  return sampledColor;
}

// Point
PointLight::PointLight(vec3 pos, vec3 color, float intensity)
    : position(pos), color(color), intensity(intensity) {}
LightType PointLight::lightType() const { return POINT; }
vec3 PointLight::sample(const Intersection &intersection,
                        const Scene &scene) const {
  vec3 sampledColor{.0f};
  auto &material = intersection.face->material;

  // Step 1: Check material properties (basically albedo != 0)
  if (material->baseColor != vec3{.0f}) {
    const vec3 lightDir = normalize(position - *intersection.pos);
    const vec3 normal = *intersection.shadingNormal;

    // Step 2: Check for occlusion. If L . N > 0, then the light is in front of
    // the surface. Since we're not modelling transmitting materials (yet), we
    // can skip all the calculations in that case.
    const float LdotN = dot(lightDir, normal);
    if (LdotN > 0) {
      // Step 3: Calculate the shadow ray. This will check for occlusion, and
      // determine if this area should be shaded by the light.
      Ray shadowRay(*intersection.pos, lightDir);
      shadowRay.adjustOrigin(normal);
      // This will also be needed, so that we don't bother checking things that
      // are further away than the light.
      const float lightDistance = distance(position, *intersection.pos);

      // Now we check if the light is visible from the face
      if (scene.visibility(shadowRay.origin, shadowRay.direction,
                           lightDistance - 0.0001f)) {
        const vec3 viewDir = -intersection.ray;

        // Step 3: Calculate the attenuation with the inverse square law
        // attenuation = 1/distance²
        const float attenuation = 1 / (lightDistance * lightDistance);

        sampledColor = material->baseColor * LdotN * intensity * attenuation;
      }
    }
  }

  return sampledColor;
}
vec3 PointLight::sample_Li(const Intersection &intersection, vec3 &wi,
                           const Scene &scene, float &pdf) const {
  const vec3 lightDir = normalize(position - *intersection.pos);
  const vec3 normal = *intersection.shadingNormal;
  vec3 sampledIntensity{.0f};
  wi = lightDir;
  pdf = 1.f;

  // Step 1: Check for occlusion. If L . N > 0, then the light is in front of
  // the surface. Since we're not modelling transmitting materials (yet), we
  // can skip all the calculations in that case.
  const float LdotN = dot(lightDir, normal);
  if (LdotN > 0) {
    // Step 2: Calculate the shadow ray. This will check for occlusion, and
    // determine if this area should be shaded by the light.
    Ray shadowRay(*intersection.pos, lightDir);
    shadowRay.adjustOrigin(normal);
    // This will also be needed, so that we don't bother checking things that
    // are further away than the light.
    const float lightDistance = distance(position, *intersection.pos);

    // Now we check if the light is visible from the face
    if (scene.visibility(shadowRay.origin, shadowRay.direction,
                         lightDistance - 0.0001f)) {
      const float attenuation = 1 / (lightDistance * lightDistance);
      sampledIntensity = color * intensity * attenuation;
    }
  }

  return sampledIntensity;
}

// Area
AreaLight::AreaLight(vec3 color, vec3 v1, vec3 v2, vec3 v3) : color(color) {
  gem = Triangle({v1, v2, v3});
  pdf = 1.f / gem.area();
  intensity = color * pdf;
}
LightType AreaLight::lightType() const { return AREA; }
vec3 AreaLight::sampleLight(vec2 rand) const {
  float sqrt_r0 = sqrtf(rand.x);
  float alpha = 1.f - sqrt_r0;
  float beta = (1.f - rand.y) * sqrt_r0;
  float gamma = rand.y * sqrt_r0;

  vec3 point = {alpha * gem.vertices[0].x + beta * gem.vertices[1].x +
                    gamma * gem.vertices[2].x,
                alpha * gem.vertices[0].y + beta * gem.vertices[1].y +
                    gamma * gem.vertices[2].y,
                alpha * gem.vertices[0].z + beta * gem.vertices[1].z +
                    gamma * gem.vertices[2].z};

  return point;
}
vec3 AreaLight::sample(const Intersection &intersection,
                       const Scene &scene) const {
  // TODO
  return {};
}
