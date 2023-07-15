#pragma once
#include <cstdint>

#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>

using namespace glm;

class Camera {
private:
  double angle_w;
  double angle_h;
  vec3 up{};
  vec3 lookingAt{};

  vec3 forward{}, right{};
  mat3 camera2world{};

public:
  uint32_t width;
  uint32_t height;
  Camera(uint32_t width, uint32_t height, double angle_x, double angle_y,
         vec3 up, vec3 pos, vec3 lookingAt);

  glm::vec3 getRay(uint32_t x, uint32_t y, vec2 jitter) const;

  void render();
  vec3 pos{};
};
