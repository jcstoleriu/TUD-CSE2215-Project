#include "shadows.h"
#include "disable_all_warnings.h"
// Suppress warnings in third-party code.
DISABLE_WARNINGS_PUSH()
#include <glm/geometric.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/vector_relational.hpp>
DISABLE_WARNINGS_POP()
#include <cmath>
#include <iostream>
#include <limits>
#include "scene.h"
#include "bounding_volume_hierarchy.h"
#include "draw.h"
#include "ray.h"
#include "ray_tracing.h"

#define SOFT_SHADOW_SAMPLE_COUNT (1 << 6)

bool traceShadowRay(const BoundingVolumeHierarchy& bvh, const glm::vec3& point, const glm::vec3& light, const glm::vec3& normal) {
  glm::vec3 direction = light - point;
  glm::vec3 directionn = glm::normalize(direction);
  Ray ray = Ray{point + directionn * 0.01f, directionn, glm::length(direction)};
  HitInfo hitInfo;

  // Light is not visible
  if (glm::dot(directionn, normal) < 0 || bvh.intersect(ray, hitInfo)) {
    drawRay(ray, glm::vec3{1.0f, 0.0f, 0.0f});
    return true;
  }

  drawRay(ray, glm::vec3{1.0f, 1.0f, 1.0f});
  return false;
}

bool hardShadow(const BoundingVolumeHierarchy& bvh, const HitInfo& hitInfo, const glm::vec3& point, const glm::vec3& light) {
  return traceShadowRay(bvh, point, light, hitInfo.normal);
}

float softShadow(const BoundingVolumeHierarchy& bvh, const HitInfo& hitInfo, const glm::vec3& point, const glm::vec3& light, const float& radius) {
  glm::vec3 n = point - light;
  glm::vec3 nn = glm::normalize(n);

  // Calculate the circle we intersect
  // We calculate its center, radius and normal
  float d = radius / (((glm::length(n) - radius) / radius) + 1);  // Distance from point to sphere has a relation to distance between intersecting plane and sphere center
  glm::vec3 intersect = light + d * nn;
  float rad = std::sqrt(std::pow(radius, 2) - std::pow(glm::length(intersect - light), 2));

  // We generate and trace a given amount of samples and we calculate what fraction hits
  int hit = 0;
  for (int i = 0; i < SOFT_SHADOW_SAMPLE_COUNT; i++) {
    // Generate a random ray

    // Random radius
    float rRadius = rad * std::sqrt(((float) std::rand()) / RAND_MAX);  // We use sqrt to ensure the random points are evenly distributed

    // Random vector perpendicular to the normal
    glm::vec3 tangent = glm::cross(nn, glm::vec3{-nn.z, nn.x, nn.y});
    glm::vec3 bitangent = glm::cross(nn, tangent);
    float randAngle = (((float) std::rand()) / RAND_MAX) * std::atan(1) * 8;  // Value between 0 and 2 PI
    glm::vec3 rDir = glm::normalize(tangent * std::sin(randAngle) + bitangent * std::cos(randAngle));

    // Calculate the point on the circle
    glm::vec3 r = intersect + rRadius * rDir;  // Random point

    // Shadow ray does not intersect
    if (!traceShadowRay(bvh, point, r, hitInfo.normal)) {
      hit++;
    }
  }

  // We return the fraction hit
  return ((float) hit) / SOFT_SHADOW_SAMPLE_COUNT;
}

// TODO: Allow color calc from rays hitting the hit surface from behind
glm::vec3 phongPart(const HitInfo& hitInfo, const glm::vec3& point, const glm::vec3 light, const glm::vec3& view, const glm::vec3& color) {
  glm::vec3 n = glm::normalize(hitInfo.normal);
  glm::vec3 l = glm::normalize(light - point);
  glm::vec3 v = glm::normalize(view - point);
  glm::vec3 r = glm::normalize(2 * glm::dot(l, n) * n - l);

  // Diffuse component
  glm::vec3 diffuse = hitInfo.material.kd * glm::max(glm::dot(l, n), 0.0f) * color;

  // Specular component
  glm::vec3 specular = glm::vec3{0.0f, 0.0f, 0.0f};

  if (glm::dot(l, n) > 0) {
    specular = hitInfo.material.ks * glm::pow(glm::max(glm::dot(r, v), 0.0f), hitInfo.material.shininess) * color;
  }

  return diffuse + specular;
}

// Here we are just doing the phongShading
glm::vec3 phongShading(const Scene& scene, const BoundingVolumeHierarchy& bvh, const HitInfo& hitInfo, const Ray& ray) {
  glm::vec3 point = ray.origin + ray.direction * ray.t;
  glm::vec3 color = glm::vec3{0.0f, 0.0f, 0.0f};

  // Note that we ignore ambient color

  // Point lights
  for (const PointLight& light : scene.pointLights) {
    if (!hardShadow(bvh, hitInfo, point, light.position)) {
      color += phongPart(hitInfo, point, light.position, ray.origin, light.color);
    }
  }

  // Spherical lights
  for (const SphericalLight& light : scene.sphericalLight) {
    float soft = softShadow(bvh, hitInfo, point, light.position, light.radius);
    if (soft > 0) {
      color += phongPart(hitInfo, point, light.position, ray.origin, light.color * soft);
    }
  }

  return color;
}