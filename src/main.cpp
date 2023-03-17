#define _USE_MATH_DEFINES

#include "camera.hpp"
#include "image.hpp"
#include "scene.hpp"
#include <fmt/core.h>

#include <cmath>

int main(int argc, char **argv) {
  // Check command-line arguments
  if (argc != 2) {
    fmt::println(stderr, "USAGE: {} [filename]", argv[0]);
    return -1;
  }

  auto scene = Scene::load(argv[1]);

  if (!scene.has_value()) {
    fmt::println(stderr, "Failed to load scene {}", argv[1]);
    return -1;
  }

  // TODO: O vetor up não está perpendicular com o looking at acho eu?
  Camera cam = Camera(1920, 1080, M_PI / 3, M_PI / 5, {0, -1, 0},
                      {200, 200, -800}, {250, 250, 250});

  scene.value().setCamera(cam);
  Image i = scene.value().render();

  i.writePNG("image.png");
}
