#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <glm/geometric.hpp>
DISABLE_WARNINGS_POP()
#include <random>
#include "draw.h"
#include "illumination.h"

static inline float surface_triangle(const Mesh &mesh, const Triangle &triangle) {
	glm::vec3 v0 = mesh.vertices[triangle.x].position;
	glm::vec3 v1 = mesh.vertices[triangle.y].position;
	glm::vec3 v2 = mesh.vertices[triangle.z].position;

	glm::vec3 v01 = v1 - v0;
	glm::vec3 v02 = v2 - v0;

	return glm::length(glm::cross(v01, v02)) / 2.0F;
}

std::vector<SamplePoint> generate_samples(const Scene &scene, const size_t count, const unsigned int seed) {
	// Surface calculation
	float surface = 0.0F;
	std::vector<std::tuple<float, size_t, size_t>> triangles;
	for (size_t i  = 0; i < scene.meshes.size(); i++) {
		const Mesh &mesh = scene.meshes[i];
		for (size_t j = 0; j < mesh.triangles.size(); j++) {
			const Triangle &triangle = mesh.triangles[j];
			float area = surface_triangle(mesh, triangle);
			triangles.push_back({area, i, j});
			surface += area;
		}
	}

	// RNG
	std::default_random_engine generator;
	generator.seed(seed);
	std::uniform_real_distribution<float> d1(0.0F, surface);
	std::uniform_real_distribution<float> d2(0.0F, 1.0F);

	// Sample generation
	std::vector<SamplePoint> samples;
	for (size_t i = 0; i < count; i++) {
		float rand = d1(generator);
		float cursor = 0.0F;
		for (const auto &[area, mi, ti] : triangles) {
			cursor += area;
			if (cursor >= rand) {
				const Mesh &mesh = scene.meshes[mi];
				const Triangle &triangle = mesh.triangles[ti];
				float u1 = d2(generator);
				float u2 = d2(generator);

				// Generating a point in the triangle
				const Vertex &v0 = mesh.vertices[triangle.x];
				const Vertex &v1 = mesh.vertices[triangle.y];
				const Vertex &v2 = mesh.vertices[triangle.z];

				if (u1 + u2 > 1.0F) {
					u1 = 1.0F - u1;
					u2 = 1.0F - u2;
				}

				float u3 = 1.0F - u1 - u2;

				glm::vec3 position = u1 * v0.position + u2 * v1.position + u3 * v2.position;
				glm::vec3 normal = u1 * v0.normal + u2 * v1.normal + u3 * v2.normal;

				samples.push_back({position, normal, mi, i});
				break;
			}
		}
	}

	return samples;
}

bool is_shadow(const BoundingVolumeHierarchy &bvh, const glm::vec3 &point, const glm::vec3 &light, const glm::vec3 &normal) {
	glm::vec3 direction = light - point;
	glm::vec3 directionn = glm::normalize(direction);
	Ray ray = Ray{point + directionn * 0.01F, directionn, glm::length(direction)};

	HitInfo hitInfo;

	// Light is not visible
	if (glm::dot(directionn, normal) < 0.0F || bvh.intersect(ray, hitInfo)) {
		drawRay(ray, glm::vec3(1.0F, 0.0F, 0.0F));
		return true;
	}

	drawRay(ray, glm::vec3(1.0F, 1.0F, 1.0F));
	return false;
}

glm::vec3 shader_lambert(const glm::vec3 &position, const glm::vec3 &normal, const Material &material, const PointLight &light) {
	float factor = glm::dot(glm::normalize(light.position - position), glm::normalize(normal));
	glm::vec3 color = factor * material.kd * light.color;
	return glm::clamp(color, 0.0F, 1.0F);
}

glm::vec3 shader_blinn_phong_specular(const glm::vec3 &position, const glm::vec3 &normal, const Material &material, const PointLight &light, const glm::vec3 &camera) {
	glm::vec3 nn = glm::normalize(normal);
	glm::vec3 v = glm::normalize(light.position - position);
	if (glm::dot(nn, v) <= 0.0F) {
		return glm::vec3(0.0F);
	}
	float factor = glm::dot(glm::normalize(v + glm::normalize(camera - position)), nn);
	glm::vec3 color = std::powf(factor, material.shininess) * material.ks * light.color;
	return glm::clamp(color, 0.0F, 1.0F);
}

glm::vec3 shader(const Scene &scene, const BoundingVolumeHierarchy &bvh, const Ray &ray, const HitInfo &hitInfo, const glm::vec3 &camera) {
	glm::vec3 point = ray.origin + ray.direction * ray.t;
	glm::vec3 color = glm::vec3(0.0F);

	for (const PointLight &light : scene.pointLights) {
		if (!is_shadow(bvh, point, light.position, hitInfo.normal)) {
			color += shader_lambert(point, hitInfo.normal, hitInfo.material, light);
			color += shader_blinn_phong_specular(point, hitInfo.normal, hitInfo.material, light, camera);
		}
	}

	return glm::clamp(color, 0.0F, 1.0F);
}
