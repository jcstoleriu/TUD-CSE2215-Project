#include "bounding_volume_hierarchy.h"
#include "draw.h"
#include "ray.h"
#include <iostream>
#include <cmath>
#include <limits>

#define BVH_SPLIT_STEPS 16


// Struct to help create the tree
struct TriangleBox {
  AxisAlignedBox box; // Triangle bounding box
  int triangle; // Triangle index
};

std::vector<Node> tree;

// Get all  nodes at a given level
std::vector<Node> getNodesAtLevel(Node& root, int level) {
  std::vector<Node> nodes;

  // Current level
  if (level == 0) {
    nodes.push_back(root);
    return nodes;
  }

  // No child values
  if (root.leaf) {
    return nodes;
  }

  for (int i = 0; i < root.indices.size(); i++) {
    for (Node node : getNodesAtLevel(tree[root.indices[i]], level - 1)) {
      nodes.push_back(node);
    }
  }
  return nodes;
}

// Get the depth of the tree
int size(Node& node) {
  // Leaf node
  if (node.leaf) {
    return 1;
  }

  int s = 0;
  for (int i = 0; i < node.indices.size(); i++) {
    s = std::max(s, 1 + size(tree[node.indices[i]]));
  }
  return s;
}

// Surface area of an AABB
float surface(const AxisAlignedBox& aabb) {
  glm::vec3 delta = aabb.upper - aabb.lower;
  return 2 * delta.x * delta.y + 2 * delta.x * delta.z + 2 * delta.y * delta.z;
}

// Resize the AABB by expanding it if necessary
void resize(AxisAlignedBox& aabb, glm::vec3 vec, bool min) {
  glm::vec3& corner = min ? aabb.lower : aabb.upper;
  corner.x = min ? std::min(corner.x, vec.x) : std::max(corner.x, vec.x);
  corner.y = min ? std::min(corner.y, vec.y) : std::max(corner.y, vec.y);
  corner.z = min ? std::min(corner.z, vec.z) : std::max(corner.z, vec.z);
}

// Create a tree from triangles
Node formTree(std::vector<TriangleBox>& triangles, int depth) {
  // Get the AABB containing all triangles
  AxisAlignedBox surround = AxisAlignedBox{glm::vec3{FLT_MAX}, glm::vec3{-FLT_MAX}};
  for (TriangleBox& tb : triangles) {
    resize(surround, tb.box.lower, true);
    resize(surround, tb.box.upper, false);
  }

  // 2 triangles per leaf max or a max depth (64 iterations)
  if (triangles.size() <= 2 || depth == (1 << 6) - 1) {
    // Create a leaf node for the triangles
    Node node;
    for (TriangleBox& tb : triangles) {
      node.indices.push_back(tb.triangle);
    }
    node.leaf = true;
    node.aabb = surround;
    return node;
  }

  // Current best cost
  float bestC = FLT_MAX;

  // Try to find the best axis to split over (x = 0, y = 1, z = 2)
  int bestAxis = -1;
  float bestSplit = -1.0f;
  for (int i = 0; i < 3; i++) {
    float l;
    float u;
    if (i == 0) {
      l = surround.lower.x;
      u = surround.upper.x;
    } else if (i == 1) {
      l = surround.lower.y;
      u = surround.upper.y;
    }
    else {
      l = surround.lower.z;
      u = surround.upper.z;
    }

    // The surrounding box does not span over this axis
    if (std::abs(u - l) < 1e-6) {
      continue;
    }

    // 16 bins evenly distributed
    float step = (u - l) / BVH_SPLIT_STEPS;

    // For each step
    for (float f = l + step; f <= u - step; f += step) {
      // Create partitions for each step
      AxisAlignedBox leftPartition = AxisAlignedBox{glm::vec3{FLT_MAX}, glm::vec3{-FLT_MAX}};
      AxisAlignedBox rightPartition = AxisAlignedBox{glm::vec3{FLT_MAX}, glm::vec3{-FLT_MAX}};
      int leftSize = 0;
      int rightSize = 0;

      // Add the triangles to the right partition
      for (TriangleBox& tb : triangles) {
        glm::vec3 center = (tb.box.lower + tb.box.upper) / 2.0f;
        float coord;
        if (i == 0) {
          coord = center.x;
        }
        else if (i == 1) {
          coord = center.y;
        }
        else {
          coord = center.z;
        }

        // See in which partition the triangle lies and resize their AABBs accordingly
        if (coord < f) {
          resize(leftPartition, tb.box.lower, true);
          resize(leftPartition, tb.box.upper, false);
          leftSize++;
        }
        else {
          resize(rightPartition, tb.box.lower, true);
          resize(rightPartition, tb.box.upper, false);
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

  // We don't find a good split so we just create one leaf node with all triangles
  // This should not really happen
  if (bestAxis == -1) {
    Node node;
    for (TriangleBox& tb : triangles) {
      node.indices.push_back(tb.triangle);
    }
    node.leaf = true;
    node.aabb = surround;
    return node;
  }

  std::vector<TriangleBox> left;
  std::vector<TriangleBox> right;

  // For each triangle assign side
  for (TriangleBox& tb : triangles) {
    glm::vec3 center = (tb.box.lower + tb.box.upper) / 2.0f;
    float coord;
    if (bestAxis == 0) {
      coord = center.x;
    }
    else if (bestAxis == 1) {
      coord = center.y;
    }
    else {
      coord = center.z;
    }

    // See in which partition the triangle lies
    if (coord < bestSplit) {
      left.push_back(tb);
    }
    else {
      right.push_back(tb);
    }
  }

  
  // Insert the partition nodes into the tree vector and save the indexes for our inner node
  Node node;
  Node leftNode = formTree(left, depth + 1);
  Node rightNode = formTree(right, depth + 1);
  int insertIndex = tree.size();
  tree.push_back(leftNode);
  tree.push_back(rightNode);
  node.indices.push_back(insertIndex);
  node.indices.push_back(insertIndex + 1);
  node.leaf = false;
  node.aabb = surround;
  return node;
}

// Create an AABB for a triangle
AxisAlignedBox toBox(Mesh& mesh, Triangle& triangle) {
  glm::vec3 v0 = mesh.vertices[triangle[0]].p;
  glm::vec3 v1 = mesh.vertices[triangle[1]].p;
  glm::vec3 v2 = mesh.vertices[triangle[2]].p;

  AxisAlignedBox aabb = AxisAlignedBox{glm::vec3{FLT_MAX}, glm::vec3{-FLT_MAX}};
  resize(aabb, v0, true);
  resize(aabb, v0, false);
  resize(aabb, v1, true);
  resize(aabb, v1, false);
  resize(aabb, v2, true);
  resize(aabb, v2, false);

  return aabb;
}

BoundingVolumeHierarchy::BoundingVolumeHierarchy(Scene* pScene): m_pScene(pScene) {
  tree.clear();

  std::vector<Node> roots;
  std::vector<Mesh>& meshes = pScene -> meshes;

  // We iterate this way so we know the element's indices
  for (int i = 0; i < meshes.size(); i++) {
    // We create a tree for every mesh in the scene
    std::vector<TriangleBox> boxes;
    std::vector<Triangle>& triangles = meshes[i].triangles;
    for (size_t j = 0; j < triangles.size(); j++) {
      AxisAlignedBox aabb = toBox(meshes[i], triangles[j]);
      TriangleBox tb = TriangleBox{aabb, static_cast<int>(j)};
      boxes.push_back(tb);
    }

    // One root node per mesh
   Node root = formTree(boxes, 0);

    roots.push_back(root);
  }

  // Push the root nodes last
  // This is so we can retrieve them with the mesh index
  for (Node& root : roots) {
    tree.push_back(root);
  }
}

// Use this function to visualize your BVH. This can be useful for debugging. Use the functions in
// draw.h to draw the various shapes. We have extended the AABB draw functions to support wireframe
// mode, arbitrary colors and transparency.
void BoundingVolumeHierarchy::debugDraw(int level) {
  // We assum level is in range [0, numLevels() - 1]

  // Get the nodes
  std::vector<Node> nodes;
  std::vector<Mesh>& meshes = m_pScene -> meshes;
  for (size_t i = 0; i < meshes.size(); i++) {
    // Get the nodes at the given level for each mesh
    std::vector<Node> nv = getNodesAtLevel(tree[tree.size() - meshes.size() + i], level);
    for (Node& nvn : nv) {
      nodes.push_back(nvn);
    }
  }

  // Draw AABB for every node
  for (Node& node : nodes) {
    drawAABB(node.aabb, DrawMode::Wireframe, glm::vec3{1.0f});
  }
}

int BoundingVolumeHierarchy::numLevels() const {
  // Tree vector: [node0, ..., nodeN, root0, ..., rootN]
  // The root nodes are the roots for each mesh
  int levels = 0;
  std::vector<Mesh>& meshes = m_pScene -> meshes;
  for (size_t i = 0; i < meshes.size(); i++) {
    levels = std::max(levels, size(tree[tree.size() - meshes.size() + i]));
  }

  return levels;
}

bool intersectNode(Ray& ray, HitInfo& hitInfo, size_t index, const Mesh& mesh) {
  const Node& current = tree[index];
  bool hit = false;
  float oldT = ray.t;

  // Ray does not intersect this node
  if (!intersectRayWithShape(current.aabb, ray)) {
    return false;
  }

  // Reset t for further intersect calculations
  ray.t = oldT;

  // Leaf node
  if (current.leaf) {
    // For each triangle in the node
    for (int i : current.indices) {
      const Triangle& tri = mesh.triangles[i];
      const Vertex& v0 = mesh.vertices[tri[0]];
      const Vertex& v1 = mesh.vertices[tri[1]];
      const Vertex& v2 = mesh.vertices[tri[2]];

      // Triangle intersection
      if (intersectRayWithTriangle(v0.p, v1.p, v2.p, ray, hitInfo)) {
        hitInfo.material = mesh.material;
        hit = true;
      }
    }

    return hit;
  }
  else {
    // Trace both halves
    bool leftIntersect = intersectNode(ray, hitInfo, current.indices[0], mesh);
    float leftT = ray.t;
    ray.t = oldT;
    bool rightIntersect = intersectNode(ray, hitInfo, current.indices[1], mesh);
    float rightT = ray.t;
    ray.t = oldT;

    // No intersection
    if (!leftIntersect && !rightIntersect) {
      return false;
    }

    ray.t = std::min(leftT, rightT);
    return true;
  }
}

// Return true if something is hit, returns false otherwise. Only find hits if they are closer than t stored
// in the ray and if the intersection is on the correct side of the origin (the new t >= 0). Replace the code
// by a bounding volume hierarchy acceleration structure as described in the assignment. You can change any
// file you like, including bounding_volume_hierarchy.h .
bool BoundingVolumeHierarchy::intersect(Ray& ray, HitInfo& hitInfo) const {
  bool hit = false;

  // For each mesh
  std::vector<Mesh>& meshes = m_pScene -> meshes;
  for (size_t i = 0; i < meshes.size(); i++) {
    hit |= intersectNode(ray, hitInfo, tree.size() - meshes.size() + i, meshes[i]);
  }

  // For each sphere
  // Spheres are not part of the BVH
  for (const Sphere& sphere : m_pScene -> spheres) {
    hit |= intersectRayWithShape(sphere, ray, hitInfo);
  }

  return hit;
}