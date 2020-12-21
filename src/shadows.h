#pragma once
#include "scene.h"
#include "bounding_volume_hierarchy.h"
#include "ray.h"
#include "ray_tracing.h"

glm::vec3 phongShading(const Scene& scene, const BoundingVolumeHierarchy& bvh, const HitInfo& hitInfo, const Ray& ray);