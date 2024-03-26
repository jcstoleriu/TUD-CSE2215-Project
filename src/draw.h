#pragma once

#include "ray.h"
#include "scene.h"

enum class DrawMode {
    FILLED,
    WIREFRAME
};

void drawMesh(const Mesh &mesh);

void drawSphere(const Sphere &sphere);

void drawSphere(const glm::vec3 &center, float radius, const glm::vec3 &color);

void drawAABB(const AxisAlignedBox &box, const DrawMode drawMode, const glm::vec3 &color, float transparency);

void drawScene(const Scene &scene);

void drawRay(const Ray &ray, const glm::vec3 &color);
