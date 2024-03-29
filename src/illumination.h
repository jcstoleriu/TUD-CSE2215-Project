#pragma once

#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include "bounding_volume_hierarchy.h"
#include "mesh.h"
#include "scene.h"

struct ShadingData {
	bool debug;
	int max_traces;
	int samples;
	std::vector<std::vector<std::vector<float>>> transforms;
};

bool is_shadow(const BoundingVolumeHierarchy &bvh, const glm::vec3 &point, const glm::vec3 &light, const glm::vec3 &normal, const bool debug);

void updateTransforms(ShadingData& data, float newVal, int idx, size_t i, size_t j);

glm::vec3 get_color(const glm::vec3 &camera, const Scene &scene, const BoundingVolumeHierarchy &bvh, const ShadingData &data, std::default_random_engine rng, Ray ray);
