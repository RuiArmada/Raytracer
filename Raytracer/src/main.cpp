#define _USE_MATH_DEFINES

#include "camera.hpp"
#include "image.hpp"
#include "scene.hpp"
#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include <cmath>

int main(int argc, char **argv) {
  // Check command-line arguments
  if (argc != 2) {
    fmt::println(stderr, "USAGE: {} [filename]", argv[0]);
    return -1;
  }

  auto sceneDef = SceneDef(argv[1]);
  auto scene = sceneDef.getScene();

  if (!scene.has_value()) {
    fmt::println(stderr, "Failed to load scene {}", argv[1]);
    return -1;
  }

  Image img = scene.value()->render();

  img.writePNG(sceneDef.outputFile);
}
