#pragma once

#include <glm/glm.hpp>
using namespace glm;

inline float absCosTheta(vec3 w) { return abs(w.y); }
inline float cos2theta(vec3 w) { return w.y * w.y; };
inline float cosTheta(vec3 w) { return sqrtf(cos2theta(w)); }
inline float sin2theta(vec3 w) { return std::max(0.f, 1.f - cos2theta(w)); }
inline float sinTheta(vec3 w) { return sqrtf(sin2theta(w)); }
inline float tan2theta(vec3 w) { return sin2theta(w) / cos2theta(w); }
inline float tanTheta(vec3 w) { return sinTheta(w) / cosTheta(w); }
inline float cosPhi(vec3 w) {
  float st = sinTheta(w);
  return (st == 0) ? 1 : clamp(w.x / st, -1.f, 1.f);
}
inline float sinPhi(vec3 w) {
  float st = sinTheta(w);
  return (st == 0) ? 1 : clamp(w.z / st, -1.f, 1.f);
}
inline float cos2phi(vec3 w) { return cosPhi(w) * cosPhi(w); }
inline float sin2phi(vec3 w) { return sinPhi(w) * sinPhi(w); }

inline bool sameHemisphere(vec3 w, vec3 wp) { return w.y * wp.y > 0; }

// I genuinely don't know what these do, but they are used in the Beckmann
// distribution
inline float erf(float x) {
  float a1 = 0.254829592f;
  float a2 = -0.284496736f;
  float a3 = 1.421413741f;
  float a4 = -1.453152027f;
  float a5 = 1.061405429f;
  float p = 0.3275911f;

  // Save the sign of x
  int sign = 1;
  if (x < 0)
    sign = -1;
  x = std::abs(x);

  // A&S formula 7.1.26
  float t = 1 / (1 + p * x);
  float y = 1 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t *
                    std::exp(-x * x);

  return (float)sign * y;
}

inline float erfInv(float x) {
  float w, p;
  x = clamp(x, -.99999f, .99999f);
  w = -std::log((1 - x) * (1 + x));
  if (w < 5) {
    w = w - 2.5f;
    p = 2.81022636e-08f;
    p = 3.43273939e-07f + p * w;
    p = -3.5233877e-06f + p * w;
    p = -4.39150654e-06f + p * w;
    p = 0.00021858087f + p * w;
    p = -0.00125372503f + p * w;
    p = -0.00417768164f + p * w;
    p = 0.246640727f + p * w;
    p = 1.50140941f + p * w;
  } else {
    w = std::sqrt(w) - 3;
    p = -0.000200214257f;
    p = 0.000100950558f + p * w;
    p = 0.00134934322f + p * w;
    p = -0.00367342844f + p * w;
    p = 0.00573950773f + p * w;
    p = -0.0076224613f + p * w;
    p = 0.00943887047f + p * w;
    p = 1.00167406f + p * w;
    p = 2.83297682f + p * w;
  }
  return p * x;
}