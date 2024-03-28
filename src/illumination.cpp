#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <glm/geometric.hpp>
DISABLE_WARNINGS_POP()
#include <random>
#include "draw.h"
#include "illumination.h"
#ifdef USE_OPENMP
#include <omp.h>
#endif

static constexpr const float OFFSET = 0.01F;
static constexpr const float M_PI = 3.14159265358979323846F;

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

static glm::vec3 shader(const Scene &scene, const BoundingVolumeHierarchy &bvh, const Ray &ray, const HitInfo &hitInfo, const glm::vec3 &camera, const bool debug) {
	glm::vec3 point = ray.origin + ray.direction * ray.t;
	glm::vec3 color = glm::vec3(0.0F);

	for (const PointLight &light : scene.pointLights) {
		if (!is_shadow(bvh, point, light.position, hitInfo.normal, debug)) {
			color += shader_lambert(point, hitInfo.normal, hitInfo.material, light);
			color += shader_blinn_phong_specular(point, hitInfo.normal, hitInfo.material, light, camera);
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

void updateTransforms(ShadingData& data, std::vector<float>& newVal, size_t i, size_t j) {
	data.transforms[i][j] = newVal;
}

static glm::vec3 get_color(const glm::vec3 &camera, const Scene &scene, const BoundingVolumeHierarchy &bvh, const ShadingData &data, std::default_random_engine rng, Ray ray, size_t depth, HitInfo hitInfo) {

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

	// Direct color

	glm::vec3 direct = shader(scene, bvh, ray, hitInfo, camera, data.debug);
	// If Ks is not black (glm::vec3{0, 0, 0} has magnitude 0)
	if (glm::length(hitInfo.material.ks) > 0.0F) {
		// Reflection of ray direction over the given normal
		glm::vec3 reflectionDir = glm::normalize(ray.direction - 2.0F * glm::dot(ray.direction, hitInfo.normal) * hitInfo.normal);
		Ray reflRay = Ray{position + reflectionDir * OFFSET, reflectionDir};
		glm::vec3 reflColor = get_color(position, scene, bvh, data, rng, reflRay, new_depth, hitInfo);
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
			glm::vec3 color = get_color(position, scene, bvh, data, rng, sampleRay, std::max(data.max_traces - 2, (int) new_depth), hitInfo);
			float factor = glm::dot(hitInfo.normal, dir);
			//if (sampleRay.t < std::numeric_limits<float>::max()) {
			//	factor /= sampleRay.t * sampleRay.t;
			//}
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

glm::vec3 get_color(const glm::vec3 &camera, const Scene &scene, const BoundingVolumeHierarchy &bvh, const ShadingData &data, std::default_random_engine rng, Ray ray) {
	HitInfo newHit;
	return get_color(camera, scene, bvh, data, rng, ray, 0, newHit);
}
