#pragma once

#include "distribution.hpp"

class BeckmannDistribution : public Distribution {
public:
  float alphax, alphay;

  BeckmannDistribution(float alphax, float alphay);
  ~BeckmannDistribution() override = default;

  /**
   * @brief Initialize a beckmann distribution with a roughness value, which
   * will be converted
   * @param roughness This value will be converted to alphax and alphay
   */
  explicit BeckmannDistribution(float roughness);

  [[nodiscard]] vec3 sample_wh(vec3 wo, vec2 u) const override;
  [[nodiscard]] float d(vec3 wh) const override;
  [[nodiscard]] float g(vec3 wo, vec3 wi) const override;
  [[nodiscard]] float g1(vec3 w) const override;
  [[nodiscard]] float pdf(vec3 wo, vec3 wh) const override;

private:
  [[nodiscard]] float lambda(vec3 w) const;
};