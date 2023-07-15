#include "path_tracer_shader.hpp"
#include "bsdf.hpp"
#include "fmt/core.h"
#include "ray.hpp"
#include <fmt/color.h>
#include <glm/gtc/random.hpp>
#define MAX_DEPTH 4
#define CONTINUE_P 0.5f

vec3 PathTracerShader::getColor(const Intersection &isect) {
  //  return getColorInternal(intersection, 0);
  // https://www.pbr-book.org/3ed-2018/Light_Transport_I_Surface_Reflection/Path_Tracing
  vec3 color{0.f}, beta{1.f};
  bool specularBounce = false;
  Ray ray(isect.source, isect.ray);
  auto intersection = isect;

  for (int bounces = 0;; ++bounces) {
    bool foundIntersection = intersection.pos.has_value();

    if (bounces == 0 || specularBounce) {
      if (foundIntersection) {
        //          color += beta * *intersection.lightColor;
        // TODO: contribute with intersection's emitted light
      } else {
        for (const auto &light : scene.lights) {
          // This way, if the ray hits nothing, we can sample an environment
          // light, such as ambient or an HDRI.
          color += beta * light->Le(ray.direction);
        }
      }
    }

    if (!foundIntersection ||
        (bounces >= MAX_DEPTH && glm::linearRand(0.0f, 1.0f) > CONTINUE_P))
      break;

    // TODO: It's best to only sample one light * n of lights
    BSDF bsdf(intersection);
    color += beta * uniformSampleAllLights(intersection, bsdf);

    vec3 wo = -ray.direction, wi;
    float pdf;
    vec3 f = bsdf.sampleF(wo, wi, pdf);

    if (f == vec3{0.f} || pdf == 0.f)
      break;
    beta *= f * abs(dot(wi, *intersection.shadingNormal)) / pdf;
    specularBounce = true;
    ray = Ray(*intersection.pos, normalize(wi));
    ray.adjustOrigin(*intersection.shadingNormal);
    intersection = scene.castRay(ray.origin, ray.direction);
  }

  return color;
}

vec3 PathTracerShader::uniformSampleAllLights(const Intersection &intersection,
                                              const BSDF &bsdf) const {
  vec3 color{0.f};

  for (auto light : scene.lights) {
    if (light->lightType() == POINT) {
      color += estimateDirect(intersection, *light, bsdf);
    }
  }

  return color;
}

vec3 PathTracerShader::estimateDirect(const Intersection &intersection,
                                      const Light &light,
                                      const BSDF &bsdf) const {
  vec3 ld{.0f};
  vec3 wi;
  float lightPdf = 0.f, scatteringPdf = 0.f;
  vec3 li = light.sample_Li(intersection, wi, scene, lightPdf);
  if (lightPdf > 0 && li != vec3{.0f}) {
    vec3 f = bsdf.f(-intersection.ray, wi) *
             abs(dot(wi, *intersection.shadingNormal));
    scatteringPdf = intersection.face->material->pdf(-intersection.ray, wi);

    // TODO: might need to check occlusion here?
    if (li != vec3{0.f}) {
      ld += f * li / lightPdf;
    }
  }

  return ld;
}

/*vec3 PathTracerShader::getColorInternal(const Intersection &intersection,
                                        int depth) {
  vec3 color{0, 0, 0};

  if (!intersection.pos.has_value()) {
    // We hit the background
    // TODO: Return background color!
    return color;
  }

  if (intersection.lightColor.has_value()) {
    return intersection.lightColor.value();
  }

  float rnd_russian = ((float)rand()) / ((float)RAND_MAX);
  if (depth < MAX_DEPTH || rnd_russian < CONTINUE_P) {
    auto m = intersection.face->material;
    vec3 lcolor = vec3(0, 0, 0);
    // if (m->specular != vec3(0, 0, 0))
    //   lcolor += specularReflection(intersection, *m, depth + 1);
    // if (m->diffuse != vec3(0, 0, 0)) {
    //   lcolor += diffuseReflection(intersection, *m, depth + 1);
    //   lcolor += directLighting(intersection, *m);
    // }

    // PBR model!
    // for (auto &light : scene.lights) {
    //   // Let's only deal with point lights for now
    //   if (light->lightType() != LightType::POINT)
    //     continue;
    //   auto pointLight = dynamic_cast<const PointLight *>(light);
    //   vec3 brdf{}, incidentRadiance{};
    //
    //   // Atenuation is the square of the distance to the light source
    //   // vec3 norm2 = (intersection.pos.value() - pointLight->position);
    //   // float atenuation =
    //   //     norm2.x * norm2.x + norm2.y * norm2.y + norm2.z * norm2.z;
    //   //
    //   // // This is how much light is hitting our point
    //   // incidentRadiance =
    //   //     (pointLight->intensity * pointLight->color) / atenuation;
    //   //
    //   // incidentRadiance = directLighting(intersection, light);
    //   vec3 diffuseLight = intersection.face->material->diffuse / (float)M_PI;
    //   lcolor = incidentRadiance * diffuseLight;
    //   //
    // }
    //

    lcolor = lightRadiance(intersection, *intersection.face->material);
    lcolor += diffuseReflection(intersection, *m, depth + 1);

    if (depth < MAX_DEPTH)
      color += lcolor;
    else
      color += lcolor / CONTINUE_P;
  }

  return color;
}

*//**
 * cosTheta = dot (h, v)
 * h = (l + v) / ||l + v||
 *
 * v is the view vector, aka -rayDir
 * l is the light vector, aka lightDir, aka normalize(lightPos - isect.por)
 *
 * ior is the index of refraction, material.ior
 *//*
vec3 fresnelSchlick(float cosTheta, vec3 f0) {
  return f0 + (vec3{1.0f} - f0) * powf(1 - cosTheta, 5);
}

/// Calculates the distribution function for a microfacet model
float ggxDistribution(vec3 normal, vec3 half, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = glm::max(dot(normal, half), 0.f);
  float NdotH2 = NdotH * NdotH;

  float nom = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = M_PI * denom * denom;

  return nom / denom;
}

float shlickGGX(float NdotV, float k) {
  // For now, we're only using direct lighting. When we support environment
  // maps, we should change how k is calculated. In that case, k = (a^2)/2
  float nom = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return nom / denom;
}

float geometrySmith(vec3 normal, vec3 view, vec3 light, float k) {
  float NdotV = glm::max(dot(normal, view), 0.0f);
  float NdotL = glm::max(dot(normal, light), 0.0f);
  float ggx1 = shlickGGX(NdotV, k);
  float ggx2 = shlickGGX(NdotL, k);

  return ggx1 * ggx2;
}

vec3 PathTracerShader::diffuseReflection(const Intersection &isect,
                                         const Material &material, int depth) {
  vec3 color{0, 0, 0};
  vec2 rand = glm::linearRand(vec2(0, 0), vec2(1, 1));
  float cosTheta = sqrtf(rand.y);

  vec3 dAroundZ{cosf(2.f * (float)M_PI * rand.x) * sqrtf(1.f - rand.y),
                sinf(2.f * (float)M_PI * rand.x) * sqrtf(1.f - rand.y),
                cosTheta};

  vec3 rayZ = *isect.shadingNormal;
  vec3 rayX, rayY;
  if (abs(rayZ.x) > abs(rayZ.y))
    rayX = vec3(-rayZ.z, 0, rayZ.x) / sqrtf(rayZ.x * rayZ.x + rayZ.z * rayZ.z);
  else
    rayX = vec3(0, rayZ.z, -rayZ.y) / sqrtf(rayZ.y * rayZ.y + rayZ.z * rayZ.z);
  rayY = cross(rayZ, rayX);

  vec3 rayOrigin = *isect.pos;
  vec3 rayDir = {
      dAroundZ.x * rayX.x + dAroundZ.y * rayY.x + dAroundZ.z * rayZ.x,
      dAroundZ.x * rayX.y + dAroundZ.y * rayY.y + dAroundZ.z * rayZ.y,
      dAroundZ.x * rayX.z + dAroundZ.y * rayY.z + dAroundZ.z * rayZ.z};

  Ray diffuse(rayOrigin, rayDir);
  diffuse.adjustOrigin(*isect.shadingNormal);

  auto intersection = scene.castRay(diffuse.origin, diffuse.direction);

  if (!intersection.lightColor.has_value()) {
    vec3 rColor = getColorInternal(intersection, depth + 1);
    color = (material.diffuse * rColor); // / (cosTheta / (float)M_PI);
  }

  return color;
}

vec3 PathTracerShader::lightRadiance(const Intersection &isect,
                                     const Material &material) {
  vec3 color{0, 0, 0};

  for (auto &light : scene.lights) {
    switch (light->lightType()) {
    case LightType::AMBIENT:
      color += material.ambient * dynamic_cast<AmbientLight *>(light)->color;
      break;
    case LightType::POINT:
      if (material.diffuse != vec3(0, 0, 0)) {
        auto &pointLight = dynamic_cast<const PointLight &>(*light);
        vec3 lightDir = glm::normalize(pointLight.position - *isect.pos);
        vec3 viewDir = -isect.ray;
        const float lDistance = glm::distance(pointLight.position, *isect.pos);

        // Atenuation is the square of the distance to the light source
        vec3 norm2 = (isect.pos.value() - pointLight.position);
        float atenuation =
            1 / (norm2.x * norm2.x + norm2.y * norm2.y + norm2.z * norm2.z);

        vec3 normal = *isect.shadingNormal;
        // TODO should be shading normal, but don't know where to get that...
        float cosL = dot(lightDir, normal);

        if (cosL > 0) {
          // Light is NOT behind the primitive
          Ray shadowRay(*isect.pos, lightDir);
          shadowRay.adjustOrigin(normal);

          if (scene.visibility(shadowRay.origin, shadowRay.direction,
                               lDistance - 0.0001f)) {
            vec3 half = normalize(lightDir + viewDir);
            float cosTheta = dot(half, viewDir);

            vec3 f0 = vec3{(1.0f - material.ior) / (1.0f + material.ior)};
            f0 = f0 * f0;
            f0 = mix(f0, material.diffuse, material.metallic);
            vec3 fresnel = fresnelSchlick(cosTheta, f0);
            float ggx = ggxDistribution(normal, half, material.roughness);
            float k = (material.roughness + 1.0f) *
                      (material.roughness + 1.0f) / 8.0f;

            // Calculates self-occlusion
            float geometry = geometrySmith(normal, viewDir, lightDir, k);

            vec3 radiance =
                pointLight.color * pointLight.intensity * atenuation;

            vec3 numerator = ggx * geometry * fresnel;
            float denominator = 4.0f * glm::max(dot(normal, viewDir), 0.0f) *
                                    glm::max(dot(normal, lightDir), 0.0f) +
                                0.0001f;
            vec3 specular = numerator / denominator;

            // How much the specular component affects the color
            vec3 kS = fresnel;
            // Same for diffuse
            vec3 kD = vec3(1.0f) - kS;

            kD *= 1.0 - material.metallic;

            float NdotL = glm::max(dot(normal, lightDir), 0.0f);
            color += (kD * material.albedo / (float)M_PI + specular) *
                     radiance * NdotL;
          }
        }
      }
      break;
    case LightType::AREA:
      if (material.diffuse != vec3(0, 0, 0)) {
        auto &areaLight = dynamic_cast<const AreaLight &>(*light);

        vec2 rnd = glm::linearRand(vec2(0, 0), vec2(1, 1));
        vec3 lightPoint = areaLight.sampleLight(rnd);

        vec3 lightDir = normalize(lightPoint - *isect.pos);
        float lightDistance = distance(*isect.pos, lightPoint);

        // cos between lightDir and the shading normal
        float cosL = dot(lightDir, *isect.shadingNormal);
        // cos between lightDir and area light normal
        float cosL_LA = dot(lightDir, areaLight.gem.planeNormal);

        if (cosL > 0 && cosL_LA <= 0) {
          Ray ray(*isect.pos, lightDir);
          ray.adjustOrigin(*isect.shadingNormal);

          if (scene.visibility(ray.origin, ray.direction,
                               lightDistance - 0.0001f))
            color +=
                (material.diffuse * areaLight.intensity * cosL) / areaLight.pdf;
        }
      }
      break;
    default:
      break;
    }
  }

  return color;
}

vec3 PathTracerShader::specularReflection(const Intersection &isect,
                                          const Material &material, int depth) {
  vec3 rayDir = reflect(isect.ray, *isect.shadingNormal);
  float cosTheta;

  if (material.shininess < 1000) {
    vec2 rnd = glm::linearRand(vec2(0, 0), vec2(1, 1));
    cosTheta = powf(rnd.y, 1 / (material.shininess + 1));

    float auxRand1 = powf(rnd.y, 2 / (material.shininess + 1));
    vec3 sAroundN{
        cosf(2.f * (float)M_PI * rnd.x) * sqrtf(1 - auxRand1),
        sinf(2.f * (float)M_PI * rnd.x) * sqrtf(1 - auxRand1),
        cosTheta,
    };

    // The greater the shininess value, the more sAroundN will point up

    vec3 rayX, rayY;
    if (abs(rayDir.x) > abs(rayDir.y)) {
      rayX = vec3(-rayDir.z, 0, rayDir.x) /
             sqrtf(rayDir.x * rayDir.x + rayDir.z * rayDir.z);
    } else {
      rayX = vec3(0, rayDir.z, -rayDir.y) /
             sqrtf(rayDir.y * rayDir.y + rayDir.z * rayDir.z);
    }
    rayY = cross(rayDir, rayX);
    rayDir = {sAroundN.x * rayX.x + sAroundN.x * rayY.x + sAroundN.z * rayDir.x,
              sAroundN.x * rayX.y + sAroundN.x * rayY.y + sAroundN.z * rayDir.y,
              sAroundN.x * rayX.z + sAroundN.x * rayY.z +
                  sAroundN.z * rayDir.z};
  }

  // Ideal specular
  Ray ray(*isect.pos, rayDir);
  ray.adjustOrigin(*isect.shadingNormal);

  auto intersection = scene.castRay(ray.origin, ray.direction);
  vec3 color = getColorInternal(intersection, depth);

  if (material.shininess < 1000 && false) {
    // Isto é suposto ajustar as cores para que tenham o peso probabilistico
    // certo, mas na verdade acabam só a desaparecer... TODO
    float power = powf(cosTheta, material.shininess) / (2.f * (float)M_PI);

    // TODO: Há um bug aqui, isto está a ficar demasiado escuro
    float pdf = (material.shininess + 1.0f) * power;
    color = (material.specular * color * power) / pdf;
  } else {
    color *= material.specular;
  }

  return color;
}
*/
