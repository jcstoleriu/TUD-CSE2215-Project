#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <glm/geometric.hpp>
DISABLE_WARNINGS_POP()
#include "draw.h"
#include "illumination.h"

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

static bool isShadow(const BoundingVolumeHierarchy &bvh, const glm::vec3 &point, const glm::vec3 &light, const glm::vec3 &normal) {
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

glm::vec3 shader(const Scene &scene, const BoundingVolumeHierarchy &bvh, const Ray &ray, const HitInfo &hitInfo, const glm::vec3 &camera) {
	glm::vec3 point = ray.origin + ray.direction * ray.t;
	glm::vec3 color = glm::vec3(0.0F);

	for (const PointLight &light : scene.pointLights) {
		if (!isShadow(bvh, point, light.position, hitInfo.normal)) {
			color += shader_lambert(point, hitInfo.normal, hitInfo.material, light);
			color += shader_blinn_phong_specular(point, hitInfo.normal, hitInfo.material, light, camera);
		}
	}

	return glm::clamp(color, 0.0F, 1.0F);
}
