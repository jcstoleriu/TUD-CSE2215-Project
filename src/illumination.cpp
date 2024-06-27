#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <glm/geometric.hpp>
DISABLE_WARNINGS_POP()
#include <iostream>
#include <random>
#include "draw.h"
#include "illumination.h"
#ifdef USE_OPENMP
#include <omp.h>
#endif

#include <vector>



static constexpr const float OFFSET = 0.01F;
static constexpr const float M_PI = 3.14159265358979323846F;
static constexpr const size_t INVALID_INDEX = (size_t) -1;

bool is_shadow(const BoundingVolumeHierarchy &bvh, const glm::vec3 &point, const glm::vec3 &light, const glm::vec3 &normal, const bool debug) {
	glm::vec3 direction = light - point;
	glm::vec3 directionn = glm::normalize(direction);
	Ray ray = Ray{point + directionn * OFFSET, directionn, glm::length(direction) - 2.0F * OFFSET};

	HitInfo hitInfo;

	// Light is not visible
	if (glm::dot(directionn, normal) < 0.0F || bvh.intersect(ray, hitInfo)) {
		if (debug) {
			drawRay(ray, glm::vec3(1.0F, 0.0F, 0.0F));
		}
		return true;
	}

	if (debug) {
		drawRay(ray, glm::vec3(0.0F, 1.0F, 1.0F));
	}
	return false;
}

static glm::vec3 shader_lambert(const glm::vec3 &position, const glm::vec3 &normal, const Material &material, const PointLight &light) {
	float factor = glm::dot(glm::normalize(light.position - position), normal);
	glm::vec3 color = factor * material.kd * light.color;
	return glm::clamp(color, 0.0F, 1.0F);
}

static glm::vec3 shader_blinn_phong_specular(const glm::vec3 &position, const glm::vec3 &normal, const Material &material, const PointLight &light, const glm::vec3 &camera) {
	glm::vec3 v = glm::normalize(light.position - position);
	if (glm::dot(normal, v) <= 0.0F) {
		return glm::vec3(0.0F);
	}
	float factor = glm::dot(glm::normalize(v + glm::normalize(camera - position)), normal);
	glm::vec3 color = std::powf(factor, material.shininess) * material.ks * light.color;
	return glm::clamp(color, 0.0F, 1.0F);
}

static glm::vec3 shader(const Scene &scene, const BoundingVolumeHierarchy &bvh, const Ray &ray, const HitInfo &hitInfo, const glm::vec3 &camera, const bool debug, TransportMatrix &transportMatrix) {
	glm::vec3 point = ray.origin + ray.direction * ray.t;
	glm::vec3 color = glm::vec3(0.0F);

	for (const PointLight &light : scene.pointLights) {
		if (!is_shadow(bvh, point, light.position, hitInfo.normal, debug)) {
			color += shader_lambert(point, hitInfo.normal, hitInfo.material, light);
			color += shader_blinn_phong_specular(point, hitInfo.normal, hitInfo.material, light, camera);

			// TODO: Accumulate transfer to transport matrix
            // float transfer = glm::dot(hitInfo.normal, light.position - point);
            // transportMatrix.matrix[hitInfo.meshIdx][light.meshIdx] += transfer;
		}
	}

	return glm::clamp(color, 0.0F, 1.0F);
}

static glm::vec3 random_hemisphere_vector(std::default_random_engine &rng) {
	std::uniform_real_distribution<float> uniform(0.0F, 1.0F);
	float u1 = uniform(rng);
	float u2 = uniform(rng);

	float st = std::sqrtf(1.0F - u1 * u1);
	float phi = 2.0F * M_PI * u2;
	float x = st * std::cosf(phi);
	float z = st * std::sinf(phi);

	return glm::vec3(x, u1, z);
}

static glm::vec3 random_hemisphere_vector(std::default_random_engine &rng, const glm::vec3 &normal) {
	glm::vec3 v = random_hemisphere_vector(rng);

	glm::vec3 nt;
	if (std::fabsf(normal.x) > std::fabsf(normal.y)) {
		nt = glm::vec3(normal.z, 0.0F, -normal.x);
	} else {
		nt = glm::vec3(0.0F, -normal.z, normal.y);
	}
	nt = glm::normalize(nt);
	glm::vec3 nb = glm::cross(normal, nt);

	float x = v.x * nb.x + v.y * normal.x + v.z * nt.x;
	float y = v.x * nb.y + v.y * normal.y + v.z * nt.y;
	float z = v.x * nb.z + v.y * normal.z + v.z * nt.z;

	return glm::vec3(x, y, z);
}

static glm::vec3 get_color(const glm::vec3 &camera, const Scene &scene, const BoundingVolumeHierarchy &bvh, const ShadingData &data, std::default_random_engine &rng, Ray &ray, TransportMatrix &transportMatrix, HitInfo &hitInfo, const size_t depth) {
	// Ray miss
	if (depth >= data.max_traces || !bvh.intersect(ray, hitInfo)) {
		// Draw a red debug ray if the ray missed.
		if (data.debug) {
			drawRay(ray, glm::vec3(1.0F, 0.0F, 0.0F));
		}

		// Set the color of the pixel to black if the ray misses.
		return glm::vec3(0.0F);
	}

	// Draw a white debug ray.
	if (data.debug) {
		drawRay(ray, glm::vec3(1.0F));
	}

	glm::vec3 position = ray.origin + ray.direction * ray.t;
	size_t new_depth = depth + 1;
	size_t sample_depth = data.max_traces < 2 ? new_depth : std::max(new_depth, (size_t) data.max_traces - 2);

	// Direct color
	TransportMatrix transport;

	glm::vec3 direct = shader(scene, bvh, ray, hitInfo, camera, data.debug, transport);
	// If Ks is not black (glm::vec3{0, 0, 0} has magnitude 0)
	if (glm::length(hitInfo.material.ks) > 0.0F) {
		// Reflection of ray direction over the given normal
		glm::vec3 reflectionDir = glm::normalize(ray.direction - 2.0F * glm::dot(ray.direction, hitInfo.normal) * hitInfo.normal);
		Ray reflRay = Ray{position + reflectionDir * OFFSET, reflectionDir};
		HitInfo new_hitInfo;
		glm::vec3 reflColor = get_color(position, scene, bvh, data, rng, reflRay, transportMatrix, new_hitInfo, new_depth);
		glm::vec3 color =  hitInfo.material.ks * reflColor;
		//if (reflRay.t < std::numeric_limits<float>::max()) {
		//	color /= reflRay.t * reflRay.t;
		//}
		direct += color;
	}

	// Indirect color

	glm::vec3 indirect = glm::vec3(0.0F);
	for (int i = 0; i < data.samples; i++) {
		glm::vec3 dir = random_hemisphere_vector(rng, hitInfo.normal);
		Ray sampleRay = Ray{position + dir * OFFSET, dir};
	
		// We only compute outside of debug draw
		if (data.debug) {
			Ray sampleRayLen1 = Ray{sampleRay.origin, sampleRay.direction, 0.1F};
			drawRay(sampleRayLen1, glm::vec3(1.0F, 1.0F, 0.0F));
		} else {
			HitInfo sample_hitInfo;
			sample_hitInfo.meshIdx = INVALID_INDEX;
			glm::vec3 color = get_color(position, scene, bvh, data, rng, sampleRay, transportMatrix, sample_hitInfo, sample_depth);
			float factor = glm::dot(hitInfo.normal, dir);

			// Transform
			// Note that the resulting color is not clamped to [0, 1] on purpose
			if (sample_hitInfo.meshIdx != INVALID_INDEX) {
				//transportMatrix.matrix[hitInfo.meshIdx][sample_hitInfo.meshIdx] += (factor / data.samples) * color;
				const auto &[scalar, offset] = (*data.transforms).get(sample_hitInfo.meshIdx, hitInfo.meshIdx);
				color = scalar * color + offset;
			}

			indirect += factor * color;
		}
	}

	if (data.samples != 0) {
		indirect /= data.samples;
		indirect *= 2.0F * M_PI;
	}

	glm::vec3 color = (direct + indirect) / M_PI; /** hitInfo.material.kd*/ /// M_PI;
	return glm::clamp(color, 0.0F, 1.0F);
}

glm::vec3 get_color(const glm::vec3 &camera, const Scene &scene, const BoundingVolumeHierarchy &bvh, const ShadingData &data, std::default_random_engine &rng, Ray &ray, TransportMatrix &transportMatrix) {
	HitInfo hitInfo;
	return get_color(camera, scene, bvh, data, rng, ray, transportMatrix, hitInfo, 0);
}

// Based on https://www.eecis.udel.edu/~amer/CISC651/wavelets_for_computer_graphics_Stollnitz.pdf
// Projects input vector into wavelet space and returns result
// Level controls how many times the pairwise averaging is applied for visualization purposes
std::vector<std::tuple<glm::vec3, glm::vec3>> haarTransformRow(const std::vector<std::tuple<glm::vec3, glm::vec3>> &row, int level) {
	// final result
	std::vector<std::tuple<glm::vec3, glm::vec3>> coeffs;
	std::vector<std::tuple<glm::vec3, glm::vec3>> vals = row;

	// can only do pairwise averaging log2(size) times
	if (level > std::log2(row.size()))
		level = std::log2(row.size());

	//if (std::pow(2, level) != row.size()) {
	//	throw std::invalid_argument("Array size is not a power of 2, which is needed for the Haar transform!");
	//}

	for (int i = 0; i < level; i++) {
		std::vector<std::tuple<glm::vec3, glm::vec3>> tempCoeffs;
		// new intermediary values
		std::vector<std::tuple<glm::vec3, glm::vec3>> temp;
		// pairwise averaging for both scale and offset
		for (int j = 0; j < vals.size(); j += 2) {
			glm::vec3 avg_scalar = (std::get<0>(vals[j]) + std::get<0>(vals[j + 1])) / glm::vec3(2.0);
			glm::vec3 avg_offset = (std::get<1>(vals[j]) + std::get<1>(vals[j + 1])) / glm::vec3(2.0);
			std::tuple<glm::vec3, glm::vec3> avg = std::tuple(avg_scalar, avg_offset);

			glm::vec3 coeff_scalar = std::get<0>(vals[j]) - std::get<0>(avg);
			glm::vec3 coeff_offset = std::get<1>(vals[j]) - std::get<1>(avg);
			std::tuple<glm::vec3, glm::vec3> coeff = std::tuple(coeff_scalar, coeff_offset);
			tempCoeffs.push_back(coeff);
			temp.push_back(avg);
		}
		// insert new coefficients at the beginning
		coeffs.insert(coeffs.begin(), tempCoeffs.begin(), tempCoeffs.end());
		vals = temp;
	}
	// insert final values at the beginning
	coeffs.insert(coeffs.begin(), vals.begin(), vals.end());

	return coeffs;
}

/*
Reconstruct original vector from its Haar-projected vector
*/
std::vector<std::tuple<glm::vec3, glm::vec3>> haarInvTransformRow(const std::vector<std::tuple<glm::vec3, glm::vec3>> &projected, int level) {

	if (level > std::log2(projected.size()))
		level = std::log2(projected.size());

	// number of values that are not coefficients in projected array (depends of level of projection)
	int nrVals = projected.size() / pow(2, level);
	std::vector<std::tuple<glm::vec3, glm::vec3>> reconstructed = { projected.begin(), projected.begin() + nrVals };

	// iteratively reconstruct original array. could also do this recursively
	// go until we have the original number of elements in the reconstructed array
	while (nrVals < projected.size()) {
		std::vector<std::tuple<glm::vec3, glm::vec3>> temp;
		for (int j = 0; j < nrVals; j++) {
			glm::vec3 first_scalar = std::get<0>(reconstructed[j]) + std::get<0>(projected[nrVals + j]);
			glm::vec3 first_offset = std::get<1>(reconstructed[j]) + std::get<1>(projected[nrVals + j]);
			std::tuple<glm::vec3, glm::vec3> first = std::tuple(first_scalar, first_offset);

			glm::vec3 second_scalar = std::get<0>(reconstructed[j]) - std::get<0>(projected[nrVals + j]);
			glm::vec3 second_offset = std::get<1>(reconstructed[j]) - std::get<1>(projected[nrVals + j]);
			std::tuple<glm::vec3, glm::vec3> second = std::tuple(second_scalar, second_offset);
			temp.push_back(first);
			temp.push_back(second);
		}
		nrVals *= 2;
		reconstructed = temp;
	}

	return reconstructed;
}
