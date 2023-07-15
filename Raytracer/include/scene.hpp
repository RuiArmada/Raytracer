#pragma once

#include "camera.hpp"
#include "geometry.hpp"
#include "image.hpp"
#include "intersection.hpp"
#include "light.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <optional>
#include <string>
#include <tiny_obj_loader.h>

using namespace glm;

class Scene {
private:
  std::vector<Object> objects;

  const Camera *camera = nullptr;

public:
  tinyobj::attrib_t attributes;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<Material> materials{};
  std::vector<Light *> lights;

  float exposure = 1;
  int samplesPerPixel = 16;

  /**
   * Loads and creates a scene from a filename
   */
  static std::optional<unique_ptr<Scene>> load(const std::string &filename);
  ~Scene();

  void setCamera(const Camera &camera);

  [[nodiscard]] Intersection castRay(vec3 origin, vec3 direction) const;
  [[nodiscard]] bool visibility(vec3 origin, vec3 direction, float L) const;

  Image render();
};

class SceneDef {
public:
  std::string modelFile, outputFile;
  uint32_t width, height;
  shared_ptr<Camera> camera;
  std::vector<Light *> lights{};

  float exposure = 1;
  int samplesPerPixel = 16;

  explicit SceneDef(const std::string &filename);

  optional<unique_ptr<Scene>> getScene();
};
