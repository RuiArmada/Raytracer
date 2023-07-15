#pragma once

#include "material.hpp"
#include <array>
#include <optional>
#include <tiny_obj_loader.h>
#include <vector>

#include <glm/vec3.hpp>

using namespace std;
using namespace glm;

class Triangle {
public:
  Triangle() = default;
  Triangle(array<vec3, 3> vertices, optional<array<vec3, 3>> normals,
           optional<array<vec2, 3>> texcoords,
           const Material *material);
  ;

  explicit Triangle(array<vec3, 3> vertices);
  /**
   * The three vertices that make up triangle.
   */
  array<vec3, 3> vertices;

  vec3 planeNormal{};

  /**
   * The three normal vectors for each vertex that
   * are needed for interpolated normal calculation in interpolated
   * shading.
   */
  std::optional<array<vec3, 3>> normals;
  std::optional<array<vec2, 3>> texcoords;

  const Material *material = nullptr;

  /**
   * Checks if a ray intersects the triangle
   * @param ray The ray vector
   * @param origin The origin point of the ray
   * @return The intersection point if an intersection was detected. Otherwise,
   * returns empty optional.
   */
  [[nodiscard]] optional<vec3> intersects(vec3 ray, vec3 origin) const;

  float area() const;
  vec3 tangent;
  vec3 bitangent;
};

class Object {
private:
  vec3 minPoint{0}, maxPoint{0};

public:
  vector<Triangle> faces;
  string name;

  /**
   * Checks if a ray intersects the object's bounding box
   * @param ray The ray vector
   * @param origin The origin point of the ray
   * @param rayInverse Inverse of the ray
   * @return Whether an intersection was detected
   */
  inline bool intersects(vec3 ray, vec3 origin, vec3 rayInverse) const {
    float tx1 = (minPoint.x - origin.x) * rayInverse.x;
    float tx2 = (maxPoint.x - origin.x) * rayInverse.x;

    float tmin = std::min(tx1, tx2);
    float tmax = std::max(tx1, tx2);

    float ty1 = (minPoint.y - origin.y) * rayInverse.y;
    float ty2 = (maxPoint.y - origin.y) * rayInverse.y;

    tmin = std::max(tmin, std::min(ty1, ty2));
    tmax = std::min(tmax, std::max(ty1, ty2));

    float tz1 = (minPoint.z - origin.z) * rayInverse.z;
    float tz2 = (maxPoint.z - origin.z) * rayInverse.z;

    tmin = std::max(tmin, std::min(tz1, tz2));
    tmax = std::min(tmax, std::max(tz1, tz2));

    return tmax >= tmin;
  }

  /**
   * Calculates the bounding box for the object.
   */
  void calculateBoundingBox();
};

class Geometry {
public:
  Geometry() = default;

  vector<Triangle> triangles;
  Material material;
};
