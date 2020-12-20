#pragma once
#include "scene.h"
#include "bounding_volume_hierarchy.h"
#include "ray.h"
#include "ray_tracing.h"

bool hardShadows(const Scene& scene, const BoundingVolumeHierarchy& bvh, Ray ray);