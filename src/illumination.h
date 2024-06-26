#pragma once

#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
#include <random>
DISABLE_WARNINGS_POP()
#include "bounding_volume_hierarchy.h"
#include "mesh.h"
#include "scene.h"
#include "sparseMatrix.h"

// Define a structure to store transport matrix
struct TransportMatrix {
    std::vector<std::vector<glm::vec3>> matrix;

	TransportMatrix() = default;

    // Initialize the transport matrix with zeros
    TransportMatrix(size_t size) : matrix(size, std::vector<glm::vec3>(size, glm::vec3(0.0F, 0.0F, 0.0F))) {}

	TransportMatrix(size_t x, size_t y) : matrix(x, std::vector<glm::vec3>(y, glm::vec3(0.0F, 0.0F, 0.0F))) {}
};

struct ShadingData {
	bool debug;
	int max_traces;
	int samples;
	//std::vector<std::vector<std::tuple<glm::vec3, glm::vec3>>> *transforms;
	SparseMatrix *transforms;
};

bool is_shadow(const BoundingVolumeHierarchy &bvh, const glm::vec3 &point, const glm::vec3 &light, const glm::vec3 &normal, const bool debug);

// TODO: pass T into get color for updates
glm::vec3 get_color(const glm::vec3 &camera, const Scene &scene, const BoundingVolumeHierarchy &bvh, const ShadingData &data, std::default_random_engine &rng, Ray &ray, TransportMatrix &transportMatrix);

std::vector<std::tuple<glm::vec3, glm::vec3>> haarTransformRow(const std::vector<std::tuple<glm::vec3, glm::vec3>> &row);
std::vector<std::tuple<glm::vec3, glm::vec3>> haarInvTransformRow(const std::vector<std::tuple<glm::vec3, glm::vec3>> &projected);
// void computeAndVisualizeTransportMatrix(const Scene &scene, const BoundingVolumeHierarchy &bvh, TransportMatrix &transportMatrix, const Trackball &camera);