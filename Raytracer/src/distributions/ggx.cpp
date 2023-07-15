#include "distributions/ggx.hpp"

// Some helper functions
float smithGGXMaskingShadowing(vec3 wi, vec3 wo, float a2) {
  // N is the up vector, aka y-axis
  // dot((0,1,0), w) = 0*w.x + 1*w.y + 0*w.z = w.y
  float NdotL = wi.y;
  float NdotV = wo.y;

  float denomA = NdotV * sqrtf(a2 + (1.0f - a2) * NdotL * NdotL);
  float denomB = NdotL * sqrtf(a2 + (1.0f - a2) * NdotV * NdotV);

  return 2.0f * NdotL * NdotV / (denomA + denomB);
}

vec3 sphericalToCartesian(float theta, float phi) {
  return {sinf(theta) * cosf(phi), cosf(theta), sinf(theta) * sinf(phi)};
}

GGXDistribution::GGXDistribution(float roughness)
    : roughness(std::max(roughness, 0.001f)) {}

vec3 GGXDistribution::sample_wh(vec3 wo, vec2 u) const {
  float a2 = roughness * roughness;

  float cosThetaH = sqrtf(max(0.0f, (1.0f - u.x) / ((a2 - 1.0f) * u.x + 1.f)));
  float sinThetaH = sqrtf(max(0.0f, 1.0f - cosThetaH * cosThetaH));
  float phiH = u.y * M_PI_2f;

  return {sinThetaH * cosf(phiH), cosThetaH, sinThetaH * sinf(phiH)};

  float theta = acosf(sqrtf((1.0f - u.x) / ((a2 - 1.0f) * u.x + 1.0f)));
  float phi = M_2_PIf * u.y;

  return sphericalToCartesian(theta, phi);
}

float GGXDistribution::pdf(vec3 wo, vec3 wh) const { return d(wh) * wh.y; }

float GGXDistribution::g(vec3 wo, vec3 wi) const {
  float a = g1(wo);
  float b = g1(wi);
  return a * b;
}

float GGXDistribution::g1(vec3 w) const {
  float NdotV = w.y;
  float nom = NdotV;
  float denom = NdotV * (1.0f - roughness) + roughness;
  return nom / denom;
}

float GGXDistribution::d(vec3 wh) const {
  float a2 = roughness * roughness;
  float NdotH = max(std::abs(wh.y), 0.0f);
  float NdotH2 = NdotH * NdotH;

  float nom = a2;
  float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
  denom = M_PIf * denom * denom;
  return nom / denom;
}
