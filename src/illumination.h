#pragma once

#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include "bounding_volume_hierarchy.h"
#include "mesh.h"
#include "scene.h"

std::vector<glm::vec3> generate_samples(const Scene &scene, const size_t count, const unsigned int seed);

bool is_shadow(const BoundingVolumeHierarchy &bvh, const glm::vec3 &point, const glm::vec3 &light, const glm::vec3 &normal);

glm::vec3 shader_lambert(const glm::vec3 &position, const glm::vec3 &normal, const Material &material, const PointLight &light);

glm::vec3 shader_blinn_phong_specular(const glm::vec3 &position, const glm::vec3 &normal, const Material &material, const PointLight &light, const glm::vec3 &camera);

glm::vec3 shader(const Scene &scene, const BoundingVolumeHierarchy &bvh, const Ray &ray, const HitInfo &hitInfo, const glm::vec3 &camera);
