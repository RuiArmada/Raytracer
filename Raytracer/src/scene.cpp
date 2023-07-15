#include "scene.hpp"
#include "geometry.hpp"
#include "light.hpp"
#include "path_tracer_shader.hpp"
#include "shader.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <fstream>
#include <memory>
#include <optional>

#include <glm/gtc/random.hpp>
#include <nlohmann/json.hpp>
#include <tiny_obj_loader.h>

using namespace tinyobj;
using namespace glm;

SceneDef::SceneDef(const std::string &filename) {
  std::ifstream js(filename);
  auto data = nlohmann::json::parse(js);

  modelFile = data["model"];
  outputFile = data["output"];
  width = data["width"];
  height = data["height"];

  auto &cameraDef = data["frames"][0]["camera"];

  camera = make_shared<Camera>(
      Camera(width, height, cameraDef["fov"]["x"], cameraDef["fov"]["y"],
             {cameraDef["up"]["x"], cameraDef["up"]["y"], cameraDef["up"]["z"]},
             {cameraDef["position"]["x"], cameraDef["position"]["y"],
              cameraDef["position"]["z"]},
             {cameraDef["lookingAt"]["x"], cameraDef["lookingAt"]["y"],
              cameraDef["lookingAt"]["z"]}));

  for (auto &light : data["lights"]) {
    if (light["type"] == "point") {
      auto l = new PointLight(
          {light["pos"]["x"], light["pos"]["y"], light["pos"]["z"]},
          {light["color"]["r"], light["color"]["g"], light["color"]["b"]},
          light["power"]);

      fmt::print(fmt::emphasis::bold | fg(fmt::color::yellow), "[light] ");
      fmt::println("Loading point light {} {} {}", l->color.x, l->color.y,
                   l->color.z);

      lights.push_back(l);
    } else if (light["type"] == "ambient") {
      auto l = new AmbientLight(
          {light["color"]["r"], light["color"]["g"], light["color"]["b"]});
      lights.push_back(l);
    } else if (light["type"] == "area") {
      auto l = new AreaLight(
          {light["color"]["r"], light["color"]["g"], light["color"]["b"]},
          {light["v1"]["x"], light["v1"]["y"], light["v1"]["z"]},
          {light["v2"]["x"], light["v2"]["y"], light["v2"]["z"]},
          {light["v3"]["x"], light["v3"]["y"], light["v3"]["z"]});
      lights.push_back(l);
    }
  }

  if (data.contains("exposure"))
    exposure = data["exposure"];

  if (data.contains("samplesPerPixel"))
    samplesPerPixel = data["samplesPerPixel"];
}

optional<unique_ptr<Scene>> SceneDef::getScene() {
  auto ret = Scene::load(modelFile);

  if (ret) {
    ret.value()->setCamera(*camera);
    ret.value()->lights = std::move(lights);
    ret.value()->samplesPerPixel = samplesPerPixel;
    ret.value()->exposure = exposure;
  }

  return std::move(ret);
}

// https://github.com/canmom/rasteriser/blob/master/fileloader.cpp
// std::optional<Scene> Scene::load(const std::string &filename, const
// std::string mats)
std::optional<unique_ptr<Scene>> Scene::load(const std::string &filename) {
  ObjReader myObjReader;

  if (!myObjReader.ParseFromFile(filename)) {
    return std::nullopt;
  }

  auto scene = std::make_unique<Scene>();

  scene->attributes = myObjReader.GetAttrib();
  scene->shapes = myObjReader.GetShapes();

  // Materials
  for (auto &material : myObjReader.GetMaterials()) {
    scene->materials.emplace_back(material);
  }

  // Objects
  for (auto &shape : scene->shapes) {
    auto &indices = shape.mesh.indices;
    auto &mat_ids = shape.mesh.material_ids;

    fmt::print(fmt::emphasis::bold | fg(fmt::color::yellow), "[{}] ",
               shape.name);
    fmt::println("Loading {} materials, {} vertices...", mat_ids.size(),
                 indices.size());

    Object obj{};

    for (auto face_id = 0; face_id < shape.mesh.material_ids.size();
         face_id++) {
      array<int, 3> vertexIndices{indices[3 * face_id].vertex_index,
                                  indices[3 * face_id + 1].vertex_index,
                                  indices[3 * face_id + 2].vertex_index};

      array<int, 3> normalIndices{indices[3 * face_id].normal_index,
                                  indices[3 * face_id + 1].normal_index,
                                  indices[3 * face_id + 2].normal_index};

      array<int, 3> texcoordIndices{indices[3 * face_id].texcoord_index,
                                    indices[3 * face_id + 1].texcoord_index,
                                    indices[3 * face_id + 2].texcoord_index};

      array<vec3, 3> vertices{
          vec3(scene->attributes.vertices[vertexIndices[0] * 3],
               scene->attributes.vertices[vertexIndices[0] * 3 + 1],
               scene->attributes.vertices[vertexIndices[0] * 3 + 2]),
          vec3(scene->attributes.vertices[vertexIndices[1] * 3],
               scene->attributes.vertices[vertexIndices[1] * 3 + 1],
               scene->attributes.vertices[vertexIndices[1] * 3 + 2]),
          vec3(scene->attributes.vertices[vertexIndices[2] * 3],
               scene->attributes.vertices[vertexIndices[2] * 3 + 1],
               scene->attributes.vertices[vertexIndices[2] * 3 + 2])};

      std::optional<array<vec3, 3>> normals = {};

      if (normalIndices[0] >= 0) {
        normals = {vec3(scene->attributes.normals[normalIndices[0] * 3],
                        scene->attributes.normals[normalIndices[0] * 3 + 1],
                        scene->attributes.normals[normalIndices[0] * 3 + 2]),
                   vec3(scene->attributes.normals[normalIndices[1] * 3],
                        scene->attributes.normals[normalIndices[1] * 3 + 1],
                        scene->attributes.normals[normalIndices[1] * 3 + 2]),
                   vec3(scene->attributes.normals[normalIndices[2] * 3],
                        scene->attributes.normals[normalIndices[2] * 3 + 1],
                        scene->attributes.normals[normalIndices[2] * 3 + 2])};
      }

      std::optional<array<vec2, 3>> texcoords = {};
      if (texcoordIndices[0] >= 0) {
        texcoords = {
          vec2(scene->attributes.texcoords[texcoordIndices[0] * 2],
               scene->attributes.texcoords[texcoordIndices[0] * 2 + 1]),
          vec2(scene->attributes.texcoords[texcoordIndices[1] * 2],
               scene->attributes.texcoords[texcoordIndices[1] * 2 + 1]),
          vec2(scene->attributes.texcoords[texcoordIndices[2] * 2],
               scene->attributes.texcoords[texcoordIndices[2] * 2 + 1]),
        };
      }

      int materialIndex = mat_ids[face_id];
      const Material *material = nullptr;
      if (materialIndex >= 0)
        material = &scene->materials[materialIndex];

      // TODO: texCoord
      obj.faces.emplace_back(vertices, normals, texcoords, material);
    }

    obj.calculateBoundingBox();
    obj.name = shape.name;
    scene->objects.push_back(obj);
  }

  return {std::move(scene)};
}

void Scene::setCamera(const Camera &newCamera) { this->camera = &newCamera; }

void printProgress(uint32_t height, uint32_t current) {
  fmt::print(fmt::emphasis::bold | fg(fmt::color::green), "\r[progress] ");
  fmt::print("{}% done", (float)(current + 1) / (float)height * 100);
  fflush(stdout);
}

Image Scene::render() {
  if (this->camera == nullptr) {
    throw std::runtime_error{"Camera must be set"};
  }

  fmt::print(fmt::emphasis::bold | fg(fmt::color::blue), "[info] ");
  fmt::println("Starting render");
  fmt::print(fmt::emphasis::bold | fg(fmt::color::blue), "[info] ");
  fmt::println("spp: {}", this->samplesPerPixel);

  PathTracerShader shader(*this);

  Image img{camera->width, camera->height};

  fmt::print(fmt::emphasis::bold | fg(fmt::color::blue), "[info] ");
  fmt::println("width: {} height: {}", camera->width, camera->height);

  for (uint32_t y = 0; y < camera->height; y++) {
    for (uint32_t x = 0; x < camera->width; x++) {
      vec3 finalColor = {0, 0, 0};

      float finalColorR = 0, finalColorG = 0, finalColorB = 0;

      // #pragma omp parallel for reduction(+:finalColorR)
      //  reduction(+:finalColorG) reduction(+:finalColorB)
      for (int i = 0; i < samplesPerPixel; i++) {
        vec2 jitter = glm::linearRand(vec2(0, 0), vec2(1, 1));
        auto ray = camera->getRay(x, y, jitter);
//                auto ray = camera->getRay(483, 270, jitter);

        vec3 color = shader.getColor(castRay(camera->pos, ray));
        finalColorR += color.r;
        finalColorG += color.g;
        finalColorB += color.b;
      }

      finalColor = {finalColorR, finalColorG, finalColorB};

      finalColor /= samplesPerPixel;
      finalColor *= exposure;
      finalColor /= finalColor + vec3{1.0};
      finalColor = glm::pow(finalColor, vec3(1 / 2.2f));

      img.imageData[(y * camera->width + x) * 3] = finalColor.r;
      img.imageData[(y * camera->width + x) * 3 + 1] = finalColor.g;
      img.imageData[(y * camera->width + x) * 3 + 2] = finalColor.b;
    }

    printProgress(camera->height, y);
  }

  fmt::print(fmt::emphasis::bold | fg(fmt::color::blue), "\n[info] ");
  fmt::println("Finished rendering");

  return img;
}

Intersection Scene::castRay(vec3 origin, vec3 direction) const {
  float dist = FLT_MAX;
  optional<vec3> intersection;
  optional<vec3> finalIntersection;
  optional<vec3> shadingNormal;
  optional<vec3> geometricNormal;
  optional<vec3> lightColor;
  const Triangle *intersectedFace = nullptr;
  const Object *intersectedObj = nullptr;

  vec3 rayInverse{1 / direction.x, 1 / direction.y, 1 / direction.z};
  for (auto &object : objects) {
    if (object.intersects(direction, origin, rayInverse)) {
      for (auto &face : object.faces) {
        // TODO: We need to filter out objects that are BEHIND the origin, but
        // the question here is... how?
        // 1. p = the point of the object closest, note- this may not be a
        // vertex...
        // 2. objDir = origin - p
        // 3. dot(direction, objDir) > 0 means that it's in front
        if ((intersection = face.intersects(direction, origin)).has_value()) {
          float new_dist = distance(origin, *intersection);
          if (dist > new_dist) {
            dist = new_dist;
            finalIntersection = intersection;
            intersectedFace = &face;
            intersectedObj = &object;
          }
        }
      }
    }
  }

  for (auto light : lights) {
    if (light->lightType() == LightType::AREA) {
      AreaLight &areaLight = *dynamic_cast<AreaLight *>(light);
      if ((intersection = areaLight.gem.intersects(direction, origin))
              .has_value()) {
        if (!finalIntersection.has_value()) {
          finalIntersection = intersection;
          intersectedFace = &areaLight.gem;
          lightColor = areaLight.color;
        } else {
          float new_dist = distance(origin, *intersection);
          if (dist > new_dist) {
            finalIntersection = intersection;
            intersectedFace = &areaLight.gem;
            lightColor = areaLight.color;
          }
        }
      }
    }
  }

  if (finalIntersection.has_value()) {
    vec3 normal = normalize(intersectedFace->planeNormal);
    if (dot(direction, normal) > 0) {
      normal = -normal;
    }

    shadingNormal = normal;
    geometricNormal = normal;
  }

  return {direction,       origin,     finalIntersection, shadingNormal,
          geometricNormal, lightColor, intersectedFace};
}

bool Scene::visibility(vec3 origin, vec3 direction, const float maxL) const {
  vec3 rayInverse{1 / direction.x, 1 / direction.y, 1 / direction.z};

  for (auto &object : objects) {
    if (object.intersects(direction, origin, rayInverse)) {
      for (auto &face : object.faces) {
        auto isect = face.intersects(direction, origin);
        if (isect) {
          if (distance(origin, *isect) < maxL) {
            return false;
          }
        }
      }
    }
  }

  return true;
}

Scene::~Scene() {
  // Dealocate every light
  for (auto &light : lights) {
    delete light;
  }
}
