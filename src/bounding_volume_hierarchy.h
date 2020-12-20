#pragma once
#include "ray_tracing.h"
#include "scene.h"
#include "draw.h"
#include "ray.h"

// Node structure for the BVH
struct Node {
  bool leaf; // If this is a leaf node
  std::vector<int> indices; // The indices of the child nodes in the node vector or the triangle indices if it is a leaf node
  AxisAlignedBox aabb;// The bounding box this node covers (I'm not sure if this is allowed but we can always create a bounding box for it
};

class BoundingVolumeHierarchy {
public:
    BoundingVolumeHierarchy(Scene* pScene);

    // Use this function to visualize your BVH. This can be useful for debugging.
    void debugDraw(int level);
    int numLevels() const;

    // Return true if something is hit, returns false otherwise.
    // Only find hits if they are closer than t stored in the ray and the intersection
    // is on the correct side of the origin (the new t >= 0).
    bool intersect(Ray& ray, HitInfo& hitInfo) const;

    bool intersect( Ray& ray, HitInfo& hitInfo, size_t index, const Mesh& mesh) const;

  

private:
    Scene* m_pScene;
};