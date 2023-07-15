#include "bsdf.hpp"
#include <glm/geometric.hpp>
#include <glm/gtc/random.hpp>
#include "helpers.hpp"

using namespace glm;

BSDF::BSDF(const Intersection &intersection) {
  ns = normalize(intersection.face->planeNormal);
  ss = normalize(intersection.face->tangent);
  ts = normalize(intersection.face->bitangent);

  if (*intersection.shadingNormal != ns) {
    ss = -ss;
    ts = -ts;
    ns = -ns;
  }

  material = intersection.face->material;
  geometricNormal = *intersection.geometricNormal;
}

vec3 BSDF::worldToLocal(vec3 v) const {
  return vec3{dot(v, ss), dot(v, ns), dot(v, ts)};
}

vec3 BSDF::localToWorld(vec3 v) const {
  return vec3{ss.x * v.x + ns.x * v.y + ts.x * v.z,
              ss.y * v.x + ns.y * v.y + ts.y * v.z,
              ss.z * v.x + ns.z * v.y + ts.z * v.z};
}

vec3 BSDF::f(vec3 woWorld, vec3 wiWorld) const {
  vec3 wi = worldToLocal(wiWorld), wo = worldToLocal(woWorld);
  vec3 wm = normalize(wi + wo);

  return material->reflectance(wo, wi, wm);
}

vec3 BSDF::sampleF(vec3 woWorld, vec3 &wiWorld, float &pdf) const {
  vec3 wi, wo = worldToLocal(woWorld);

  float diffuseWeight = 1.f - (material->f0.x+material->f0.y+material->f0.z)/3.f;

  vec3 f;
  if (linearRand(0.f, 1.f) < diffuseWeight) {
    f = material->sampleFDiffuse(wo, wi, pdf);
    pdf /= diffuseWeight;
  } else {
    f = material->sampleFSpecular(wo, wi, pdf);
    pdf /= 1.f-diffuseWeight;
  }

  if (pdf == 0)
    return vec3{0.f};

  wiWorld = localToWorld(wi);

  if (dot(wiWorld, ns) < 0)
    wiWorld = -wiWorld;

  float NdotL = cosTheta(wi);

  return f * NdotL;
}