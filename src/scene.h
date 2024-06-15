#pragma once

#include "mesh.h"

enum SceneType {
    CornellBox
};

struct Plane {
    float d = 0.0F;
    glm::vec3 normal { 0.0F, 1.0F, 0.0F };
};

struct AxisAlignedBox {
    glm::vec3 lower { 0.0F };
    glm::vec3 upper { 1.0F };
};

struct Sphere {
    glm::vec3 center{0.0F};
    float radius = 1.0F;
    Material material;
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    size_t meshIdx;
};

struct Scene {
    std::vector<Mesh> meshes;
    std::vector<PointLight> pointLights;
};

Scene loadScene(SceneType type, const std::filesystem::path &dataDir);
