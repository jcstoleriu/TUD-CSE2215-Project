#pragma once

#include "ray_tracing.h"
#include "scene.h"

class BoundingVolumeHierarchy {
public:
    BoundingVolumeHierarchy(const Scene *scene);

    ~BoundingVolumeHierarchy();

    void debugDraw(const size_t level) const;

    size_t numLevels() const;

    bool intersect(Ray &ray, HitInfo &hitInfo) const;

    bool intersectTriangles(Ray &ray, HitInfo &hitInfo) const;

private:
    BoundingVolumeHierarchy();

    void populateTree(const std::vector<std::tuple<AxisAlignedBox, size_t, size_t>> &boxes, size_t depth);

    const Scene *scene = NULL;
    AxisAlignedBox aabb = AxisAlignedBox{glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX)};
    std::vector<std::tuple<size_t, size_t>> indices;
    BoundingVolumeHierarchy *left = NULL;
    BoundingVolumeHierarchy *right = NULL;
};
