#pragma once

#include "distribution.hpp"

class GGXDistribution : public Distribution {
private:
  float roughness;

public:
  explicit GGXDistribution(float roughness);

  [[nodiscard]] vec3 sample_wh(vec3 wo, vec2 u) const override;
  [[nodiscard]] float d(vec3 wh) const override;
  [[nodiscard]] float g(vec3 wo, vec3 wi) const override;
  [[nodiscard]] float g1(vec3 w) const override;
  [[nodiscard]] float pdf(vec3 wo, vec3 wh) const override;
};
