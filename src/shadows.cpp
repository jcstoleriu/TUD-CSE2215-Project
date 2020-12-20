#include "shadows.h"
#include "disable_all_warnings.h"
// Suppress warnings in third-party code.
DISABLE_WARNINGS_PUSH()
#include <glm/geometric.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/vector_relational.hpp>
DISABLE_WARNINGS_POP()
#include <cmath>
#include <iostream>
#include <limits>
#include "scene.h"
#include "bounding_volume_hierarchy.h"
#include "draw.h"
#include "ray.h"
#include "ray_tracing.h"


bool hardShadows(const Scene& scene, const BoundingVolumeHierarchy& bvh, Ray ray) {
    glm::vec3 point = ray.origin + ray.t * ray.direction;
    HitInfo hitInfo;
    std::vector<std::tuple<glm::vec3, float>> colors;
    for (PointLight light : scene.pointLights) {
        glm::vec3 direction = glm::normalize(light.position - point);
        float distance = glm::length(light.position - point);

        Ray shadow = Ray{point + 0.01f * direction, direction, distance};
        if (bvh.intersect(shadow, hitInfo)) {
            //drawRay(shadow, glm::vec3(1.0f, 0.0f, 0.0f));
        }
        else {
            // At least one light is visible so no hard shadow
            colors.push_back(std::tuple<glm::vec3, float>{light.color, distance});
            //drawRay(shadow, glm::vec3{1.0f, 1.0f, 1.0f});
        }
    }

    glm::vec3 color = glm::vec3{0.0f, 0.0f, 0.0f};
    for (std::tuple<glm::vec3, float> tuple : colors) {
      color += std::get<0>(tuple) / std::pow(std::get<1>(tuple), 2);
    }
    return true;
    //color / ((float)colors.size());
}