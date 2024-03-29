#include "bounding_volume_hierarchy.h"
#include "draw.h"

static constexpr size_t BVH_SPLIT_STEPS = 16;
static constexpr size_t BVH_MAX_DEPTH = 1 << 4;
static constexpr size_t BVH_LEAF_TRIANGLE_COUNT = 2;

static inline float surface(const AxisAlignedBox &aabb) {
    glm::vec3 delta = aabb.upper - aabb.lower;
    return 2.0F * delta.x * delta.y + 2.0F * delta.x * delta.z + 2.0F * delta.y * delta.z;
}

static inline void resize(AxisAlignedBox &aabb, const glm::vec3 &vec) {
    for (size_t i = 0; i < 3; i++) {
        aabb.lower[i] = std::min(aabb.lower[i], vec[i]);
        aabb.upper[i] = std::max(aabb.upper[i], vec[i]);
    }
}

static inline AxisAlignedBox toBox(const Mesh &mesh, const Triangle &triangle) {
    AxisAlignedBox aabb = AxisAlignedBox{glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX)};
    for (size_t i = 0; i < 3; i++) {
        resize(aabb, mesh.vertices[triangle[i]].position);
    }
    return aabb;
}

BoundingVolumeHierarchy::BoundingVolumeHierarchy(const Scene *scene) {
    this->scene = scene;

    std::vector<std::tuple<AxisAlignedBox, size_t, size_t>> boxes;
    const std::vector<Mesh> &meshes = scene->meshes;
    for (size_t i = 0; i < meshes.size(); i++) {
        const Mesh &mesh = meshes[i];
        const std::vector<Triangle> &triangles = mesh.triangles;

        for (size_t j = 0; j < triangles.size(); j++) {
            AxisAlignedBox box = toBox(meshes[i], triangles[j]);
            boxes.push_back({box, i, j});
        }
    }

    populateTree(boxes, 0);
}

BoundingVolumeHierarchy::~BoundingVolumeHierarchy() {
    if (left != NULL) {
        delete left;
    }
    if (right != NULL) {
        delete right;
    }
}

void BoundingVolumeHierarchy::debugDraw(const size_t level) const {
    // Correct level
    if (level == 0) {
        drawAABB(aabb, DrawMode::WIREFRAME, glm::vec3(1.0F), 1.0F);
        return;
    }

    size_t new_level = level - 1;

    if (left != NULL) {
        left->debugDraw(new_level);
    }
    if (right != NULL) {
        right->debugDraw(new_level);
    }
}

size_t BoundingVolumeHierarchy::numLevels() const {
    size_t ll = left == NULL ? 0 : left->numLevels();
    size_t rl = right == NULL ? 0 : right->numLevels();
    return 1 + std::max(ll, rl);
}

bool BoundingVolumeHierarchy::intersect(Ray &ray, HitInfo &hitInfo) const {
    // Ray does not intersect this node
    // There is a chance the ray is inside the bounding box, so it does not intersect any face
    // This copy of the ray goes off to infinity, so it must intersect a face if the original ray is inside the bounding box
    Ray copy{ray.origin, ray.direction};
    if (!intersectRayWithShape(aabb, copy)) {
        return false;
    }

    // These bools are all marked as const so they are forcibly evaluated
    // This is because we care about the closest triangle we intersect, not just any triangle
    const bool lh = left == NULL ? false : left->intersect(ray, hitInfo);
    const bool rh = right == NULL ? false : right->intersect(ray, hitInfo);
    const bool hit = intersectTriangles(ray, hitInfo);

    return lh || rh || hit;
}

bool BoundingVolumeHierarchy::intersectTriangles(Ray &ray, HitInfo &hitInfo) const {
    bool hit = false;

    // Triangles in this hierarchy
    for (const auto [mi, ti] : indices) {
        const Mesh &mesh = scene->meshes[mi];
        const Triangle &triangle = mesh.triangles[ti];
        const Vertex &v0 = mesh.vertices[triangle[0]];
        const Vertex &v1 = mesh.vertices[triangle[1]];
        const Vertex &v2 = mesh.vertices[triangle[2]];
        if (intersectRayWithTriangle(v0.position, v1.position, v2.position, ray, hitInfo)) {
            hitInfo.material = mesh.material;
            hitInfo.meshIdx = mi;
            hit = true;
        }
    }

    return hit;
}

BoundingVolumeHierarchy::BoundingVolumeHierarchy() {
    // Nothing
}

void BoundingVolumeHierarchy::populateTree(const std::vector<std::tuple<AxisAlignedBox, size_t, size_t>> &boxes, size_t depth) {
    // This function should only be called on init

    // Get the AABB containing all triangles
    AxisAlignedBox surround = AxisAlignedBox{glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX)};
    for (const auto &[box, mi, ti] : boxes) {
        resize(surround, box.lower);
        resize(surround, box.upper);
    }
    aabb = surround;

    // If this needs to be a leaf
    if (boxes.size() <= BVH_LEAF_TRIANGLE_COUNT || depth == BVH_MAX_DEPTH) {
        for (const auto &[_, mi, ti] : boxes) {
            indices.push_back({mi, ti});
        }
        return;
    }

    // Current best cost
    float bestC = FLT_MAX;

    size_t bestAxis = 0;
    float bestSplit = 0.0F;

    // For each axis
    for (size_t i = 0; i < 3; i++) {
        float l = aabb.lower[i];
        float u = aabb.upper[i];

        // The surrounding box does not span over this axis
        if (std::abs(u - l) < 1E-6) {
            continue;
        }

        // Uniformly distributed bins
        float step = (u - l) / BVH_SPLIT_STEPS;

        // For each step
        for (float f = l + step; f <= u - step; f += step) {
            // Create partitions for each step
            AxisAlignedBox leftPartition = AxisAlignedBox{glm::vec3{FLT_MAX}, glm::vec3{-FLT_MAX}};
            AxisAlignedBox rightPartition = AxisAlignedBox{glm::vec3{FLT_MAX}, glm::vec3{-FLT_MAX}};

            size_t leftSize = 0;
            size_t rightSize = 0;

            // Add the triangles to the right partition
            for (const auto &[box, mi, ti] : boxes) {
                glm::vec3 center = (box.lower + box.upper) / 2.0F;
                float coord = center[i];

                // See in which partition the triangle lies and resize their AABBs accordingly
                if (coord < f) {
                    resize(leftPartition, box.lower);
                    resize(leftPartition, box.upper);
                    leftSize++;
                } else {
                    resize(rightPartition, box.lower);
                    resize(rightPartition, box.upper);
                    rightSize++;
                }
            }

            // Partitions with no triangles are ignored
            if (leftSize == 0 || rightSize == 0) {
                continue;
            }

            // Calculate the partition cost
            float cost = leftSize * (surface(leftPartition) / surface(surround)) + rightSize * (surface(rightPartition) / surface(surround));

            // New best cost
            if (cost < bestC) {
                bestC = cost;
                bestSplit = f;
                bestAxis = i;
            }
        }
    }

    // We do not find a good split so we just create one leaf node with all triangles
    // This should not really happen
    if (bestC == FLT_MAX) {
        for (const auto &[box, mi, ti] : boxes) {
            indices.push_back({mi, ti});
        }
        return;
    }

    std::vector<std::tuple<AxisAlignedBox, size_t, size_t>> lb;
    std::vector<std::tuple<AxisAlignedBox, size_t, size_t>> rb;

    // For each triangle assign side
    for (const auto &[box, mi, ti] : boxes) {
        glm::vec3 center = (box.lower + box.upper) / 2.0F;
        float coord = center[bestAxis];

        // See in which partition the triangle lies
        if (coord < bestSplit) {
            lb.push_back({box, mi, ti});
        } else {
            rb.push_back({box, mi, ti});
        }
    }

    size_t new_depth = depth + 1;
    left = new BoundingVolumeHierarchy();
    left->scene = scene;
    left->populateTree(lb, new_depth);
    right = new BoundingVolumeHierarchy();
    right->scene = scene;
    right->populateTree(rb, new_depth);
}
