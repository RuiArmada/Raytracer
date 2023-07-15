#include "distributions/beckmann.hpp"
#include "helpers.hpp"
#include <glm/gtc/random.hpp>

void beckmannSample11(float cosThetaI, float U1, float U2, float *slope_x,
                      float *slope_y) {
  /* Special case (normal incidence) */
  if (cosThetaI > .9999) {
    float r = std::sqrt(-std::log(1.0f - U1));
    float sinPhi = std::sin(2 * M_PIf * U2);
    float cosPhi = std::cos(2 * M_PIf * U2);
    *slope_x = r * cosPhi;
    *slope_y = r * sinPhi;
    return;
  }

  /* The original inversion routine from the paper contained
     discontinuities, which causes issues for QMC integration
     and techniques like Kelemen-style MLT. The following code
     performs a numerical inversion with better behavior */
  float sinThetaI =
      std::sqrt(std::max((float)0, (float)1 - cosThetaI * cosThetaI));
  float tanThetaI = sinThetaI / cosThetaI;
  float cotThetaI = 1 / tanThetaI;

  /* Search interval -- everything is parameterized
     in the Erf() domain */
  float a = -1, c = erf(cotThetaI);
  float sample_x = std::max(U1, (float)1e-6f);

  /* Start with a good initial guess */
  // float b = (1-sample_x) * a + sample_x * c;

  /* We can do better (inverse of an approximation computed in
   * Mathematica) */
  float thetaI = std::acos(cosThetaI);
  float fit = 1 + thetaI * (-0.876f + thetaI * (0.4265f - 0.0594f * thetaI));
  float b = c - (1 + c) * std::pow(1 - sample_x, fit);

  /* Normalization factor for the CDF */
  static const float SQRT_PI_INV = 1.f / std::sqrt(M_PIf);
  float normalization =
      1 / (1 + c + SQRT_PI_INV * tanThetaI * std::exp(-cotThetaI * cotThetaI));

  int it = 0;
  while (++it < 10) {
    /* Bisection criterion -- the oddly-looking
       Boolean expression are intentional to check
       for NaNs at little additional cost */
    if (!(b >= a && b <= c))
      b = 0.5f * (a + c);

    /* Evaluate the CDF and its derivative
       (i.e. the density function) */
    float invErf = erfInv(b);
    float value =
        normalization *
            (1 + b + SQRT_PI_INV * tanThetaI * std::exp(-invErf * invErf)) -
        sample_x;
    float derivative = normalization * (1 - invErf * tanThetaI);

    if (std::abs(value) < 1e-5f)
      break;

    /* Update bisection intervals */
    if (value > 0)
      c = b;
    else
      a = b;

    b -= value / derivative;
  }

  /* Now convert back into a slope value */
  *slope_x = erfInv(b);

  /* Simulate Y component */
  *slope_y = erfInv(2.0f * std::max(U2, (float)1e-6f) - 1.0f);
}

// Helper functions!
vec3 beckmannSample(vec3 wi, float alphax, float alphay, float u1, float u2) {
  vec3 wiStretched = normalize(vec3(alphax * wi.x, wi.y, alphay * wi.z));
  float slopex, slopey;
  beckmannSample11(cosTheta(wiStretched), u1, u2, &slopex, &slopey);

  float tmp = cosPhi(wiStretched) * slopex - sinPhi(wiStretched) * slopey;
  slopey = sinPhi(wiStretched) * slopex + cosPhi(wiStretched) * slopey;
  slopex = tmp;

  slopex = alphax * slopex;
  slopey = alphay * slopey;

  return normalize(vec3(-slopex, 1.f, -slopey));
}

BeckmannDistribution::BeckmannDistribution(float alphax, float alphay)
    : alphax(alphax), alphay(alphay) {}

BeckmannDistribution::BeckmannDistribution(float roughness) {
  // Conversion function taken from pbrt
  float tmpRoughness = std::max(roughness, 1e-3f);
  float x = log(tmpRoughness);
  alphax = 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x +
           0.000640711f * x * x * x * x;
  alphay = alphax;
}

vec3 BeckmannDistribution::sample_wh(vec3 wo, vec2 u) const {
  bool flip = wo.y < 0;
  vec3 wh = beckmannSample(flip ? -wo : wo, alphax, alphay, u.x, u.y);
  if (flip)
    wh = -wh;
  return wh;
}

float BeckmannDistribution::d(vec3 wh) const {
  const float tan2Theta = tan2theta(wh);
  if (std::isinf(tan2Theta))
    return 0.f;
  float cos4theta = cos2theta(wh) * cos2theta(wh);
  return std::exp(-tan2Theta * (cos2phi(wh) / (alphax * alphax) +
                                sin2phi(wh) / (alphay * alphay))) /
         (M_PIf * alphax * alphay * cos4theta);
}

float BeckmannDistribution::g(vec3 wo, vec3 wi) const {
  return 1 / (1 + lambda(wo) + lambda(wi));
}

float BeckmannDistribution::g1(vec3 w) const { return 1 / (1 + lambda(w)); }

float BeckmannDistribution::pdf(vec3 wo, vec3 wh) const {
  return d(wh) * g1(wo) * abs(dot(wo, wh)) / absCosTheta(wo);
}

float BeckmannDistribution::lambda(vec3 w) const {
  float absTanTheta = std::abs(tanTheta(w));
  if (std::isinf(absTanTheta))
    return 0.f;
  float alpha =
      sqrtf(cos2phi(w) * alphax * alphax + sin2phi(w) * alphay * alphay);
  float a = 1 / (alpha / absTanTheta);
  if (a >= 1.6f)
    return 0;
  return (1.f - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
}
