#pragma once

#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <filesystem>
#include <vector>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
};

struct Material {
	glm::vec3 kd{1.0F};
	glm::vec3 ks{0.0F};
	float shininess{1.0F};
	float transparency{1.0F};
};

using Triangle = glm::uvec3;

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<Triangle> triangles;
	Material material;
};

[[nodiscard]] std::vector<Mesh> loadMesh(const std::filesystem::path &file, const bool normalize);
