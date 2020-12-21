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


bool hardShadows(const Scene& scene, const BoundingVolumeHierarchy& bvh, Ray ray) {
    glm::vec3 point = ray.origin + ray.t * ray.direction;
    HitInfo hitInfo;
    std::vector<std::tuple<glm::vec3, float>> colors;
    for (PointLight light : scene.pointLights) {
        glm::vec3 direction = glm::normalize(light.position - point);
        float distance = glm::length(light.position - point);

        Ray shadow = Ray{point + 0.01f * direction, direction, distance};
        if (bvh.intersect(shadow, hitInfo)) {
            //drawRay(shadow, glm::vec3(1.0f, 0.0f, 0.0f));
        }
        else {
            // At least one light is visible so no hard shadow
            colors.push_back(std::tuple<glm::vec3, float>{light.color, distance});
            //drawRay(shadow, glm::vec3{1.0f, 1.0f, 1.0f});
        }
    }

    glm::vec3 color = glm::vec3{0.0f, 0.0f, 0.0f};
    for (std::tuple<glm::vec3, float> tuple : colors) {
      color += std::get<0>(tuple) / std::pow(std::get<1>(tuple), 2);
    }
    return true;
    //color / ((float)colors.size());
}

std::tuple<float, float> sampleSphere(const BoundingVolumeHierarchy& bvh, const SphericalLight& sphere, glm::vec3& point, Ray& ray, glm::vec3& normal) {
  glm::vec3 n = point - sphere.position;
  glm::vec3 nn = glm::normalize(n);

  // Calculate the circle we intersect
  // We calculate its center, radius and normal
  float d = sphere.radius / (((glm::length(n) - sphere.radius) / sphere.radius) + 1);  // Distance from point to sphere has a relation to distance between intersecting plane and sphere center
  glm::vec3 intersect = sphere.position + d * nn;
  float rad = std::sqrt(std::pow(sphere.radius, 2) - std::pow(glm::length(intersect - sphere.position), 2));

  // We generate and trace a given amount of samples and we calculate what
  // fraction hits
  int hit = 0;
  for (int i = 0; i < SOFT_SHADOW_SAMPLE_COUNT; i++) {
    // Generate a random ray
    // Random radius
    float rRadius = rad * std::sqrt(((float) std::rand()) / RAND_MAX);  // We use sqrt to ensure the random points are evenly destributed

    // Random vector perpendicular to the normal
    glm::vec3 tangent = glm::cross(nn, glm::vec3{-nn.z, nn.x, nn.y});
    glm::vec3 bitangent = glm::cross(nn, tangent);
    float randAngle = (((float)std::rand()) / RAND_MAX) * std::atan(1) * 8;  // Value between 0 and 2 PI
    glm::vec3 rDir = tangent * std::sin(randAngle) + bitangent * std::cos(randAngle);

    // Calculate the point on the circle
    glm::vec3 r = intersect + rRadius * glm::normalize(rDir);  // Random point

    // TODO: Check if ray comes from behind the surface the point lies on

    Ray shadowRay = Ray{point + glm::normalize(r - point) * 0.01f, r - point, 1.0f};  // Small offset

    // Turn the normal if it faces away from the ligt source
    if (glm::dot(glm::normalize(ray.direction), normal) > 0) {
      normal *= -1;
    }

    HitInfo hitInfo;
    bvh.intersect(shadowRay, hitInfo);  // We ignore the return value
    // If we intersect t will be modified, if we hit the light t = 1
    // We also draw a visual ray
    // Floating point error correction
    if (std::abs(shadowRay.t - 1.0f) <= 1e-6 && glm::dot(glm::normalize(shadowRay.direction), normal) > 0) {
      hit++;
      drawRay(shadowRay, glm::vec3{1.0f, 1.0f, 1.0f});
    }
    else {
      drawRay(shadowRay, glm::vec3{1.0f, 0.0f, 0.0f});
    }
  }

  // We return the fraction hit and the distance
  // Note that this is the distance from the light center to the point
  return std::tuple{((float) hit) / SOFT_SHADOW_SAMPLE_COUNT, glm::length(n)};
}

glm::vec3 softShadows(const Scene& scene, const BoundingVolumeHierarchy& bvh, Ray& ray, glm::vec3 normal) {
  glm::vec3 point = ray.origin + ray.t * ray.direction;
  // Tuple: color, fraction, distance
  std::vector<std::tuple<glm::vec3, float, float>> results;

  // Get color, fraction visible and distance for each light source
  for (const SphericalLight& sphericalLight : scene.sphericalLight) {
    std::tuple<float, float> sampleResult = sampleSphere(bvh, sphericalLight, point, ray, normal);
    results.push_back(std::tuple{sphericalLight.color, std::get<0>(sampleResult), std::get<1>(sampleResult)});
  }

  // Average
  glm::vec3 sum = glm::vec3{0, 0, 0};
  for (std::tuple<glm::vec3, float, float>& result : results) {
    sum += (std::get<0>(result) * std::get<1>(result) / std::pow(std::get<2>(result), 2));
  }
  return sum / ((float) results.size());
}

// Here we are just doing the phongShading
glm::vec3 phongShading(const Scene& scene, const HitInfo hitInfo, const Ray& ray) {
  glm::vec3 hit = ray.origin + ray.t * ray.direction;

  glm::vec3 normal = glm::normalize(hitInfo.normal);
  glm::vec3 colour(0, 0, 0);
  for (PointLight lights : scene.pointLights) {
    glm::vec3 light = lights.position - hit;
    light = glm::normalize(light);

    // calculate the diffuse
    glm::vec3 diffuse = hitInfo.material.kd *
                        glm::max(glm::dot(normal, light), 0.0f) * lights.color;

    glm::vec3 view = glm::normalize(ray.origin - hit);
    float dot1 = glm::dot(normal, light);
    float dot2 = glm::dot(normal, view);
    if (dot1 * dot2 <= 0.0f) {
      continue;
    }
    glm::vec3 reflected = glm::reflect(-light, normal);
    float spec = glm::pow(glm::max(glm::dot(view, reflected), 0.0f),
                          hitInfo.material.shininess);
    // calculating the specular
    glm::vec3 specular = hitInfo.material.ks * spec;

    colour += (diffuse + specular);
  }

  for (SphericalLight sphericalLight : scene.sphericalLight) {
    glm::vec3 light = sphericalLight.position - hit;
    light = glm::normalize(light);
    glm::vec3 view = glm::normalize(ray.origin - hit);
    float dot1 = glm::dot(normal, light);
    float dot2 = glm::dot(normal, view);
    if (dot1 * dot2 <= 0.0f) {
      continue;
    }

    // calculate the diffuse
    glm::vec3 diffuse = hitInfo.material.kd *
                        glm::max(glm::dot(normal, light), 0.0f) *
                        sphericalLight.color;
    colour += diffuse;
  }

  return colour;
}