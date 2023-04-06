#include "shader.hpp"
#include <fmt/core.h>
#include <glm/gtc/random.hpp>

vec3 AmbientShader::getColor(const Intersection &intersection) {
  vec3 color{0};
  if (intersection.face) {
    vec3 curLight =
        ambientLight *
        glm::max(0.2f, dot(-intersection.ray,
                           normalize(intersection.face->planeNormal)));
    if (intersection.face->material) {
      color.r += intersection.face->material->diffuse[0] * curLight.r;
      color.g += intersection.face->material->diffuse[1] * curLight.g;
      color.b += intersection.face->material->diffuse[2] * curLight.b;
    }
  }
  return color;
}

vec3 RayCastShader::getColorInternal(const Intersection &intersection, int i) {
  vec3 color{0};
  int samples = 8;

  if (i < 3 && intersection.face && intersection.face->material) {
    // TODO: We need to get the roughness of the material, and randomize the ray
    // direction
    float roughness = 0.5;
    auto mtl = intersection.face->material;

    if (!mtl) {
      return color;
    }

    vec3 newRay = reflect(intersection.ray, intersection.face->planeNormal);

    vec3 reflectedColor{0};
    for (int j = 0; j < samples; j++) {
      // TODO: randomly shift the ray
      vec3 dir = glm::sphericalRand(1.0);
      if (dot(dir, intersection.face->planeNormal) <= 0)
        dir = -dir;
      reflectedColor +=
          getColorInternal(scene.castRay(*intersection.pos, dir), i + 1);
    }

    reflectedColor /= samples;

    color.r = reflectedColor.r * mtl->diffuse[0] + mtl->emission[0];
    color.g = reflectedColor.g * mtl->diffuse[1] + mtl->emission[1];
    color.b = reflectedColor.b * mtl->diffuse[2] + mtl->emission[2];
  }

  return color;
}
