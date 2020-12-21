#include "ray_tracing.h"
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


// Util method to compare floats with a 1e-6 error margin
bool eq(float f0, float f1) {
  return std::abs(f0 - f1) < 1e-6;
}

bool pointInTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& n, const glm::vec3& p) {
  // ~ MAGIC ~
  // This is a rewrite of P = v0 + u * (v1 - v0) + v * (v2 - v0)
  // Where
  // - v0, v1 and v2 are the triangle's vertices
  // - P is a point in the triangle IFF u >= 0, v >= 0 and u + v <= 1
  // We are solving for u and v for a given point P and checking the constraints
  glm::vec3 a = v2 - v0;
  glm::vec3 b = v1 - v0;
  glm::vec3 c = p - v0;

  float f = glm::dot(a, a);
  float g = glm::dot(a, b);
  float h = glm::dot(a, c);
  float i = glm::dot(b, b);
  float j = glm::dot(b, c);

  float denom = f * i - g * g;
  float u = (i * h - g * j) / denom;
  float v = (f * j - g * h) / denom;

  return u >= 0 && v >= 0 && u + v <= 1;
}

bool intersectRayWithPlane(const Plane& plane, Ray& ray) {
  glm::vec3 p = plane.normal * plane.D;  // Point on the plane
  float f = glm::dot(ray.direction, plane.normal);
  float g = glm::dot((p - ray.origin), plane.normal);

  // Ray is parallel to plane and not inside plane
  if (eq(f, 0) && !eq(g, 0)) {
    return false;
  }

  // Here we give t the value 0 IFF the ray is parallel and inside the plane
  // This is not a perfect solution
  float t = eq(g, 0) ? 0 : g / f;

  // Behind the camera
  if (t < 0) {
    return false;
  }

  ray.t = t;
  return true;
}

Plane trianglePlane(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
  glm::vec3 a = v0 - v2;
  glm::vec3 b = v1 - v2;
  glm::vec3 normal = glm::normalize(glm::cross(a, b));
  float d = glm::dot(normal, v0);

  // Note that d can be negative so we invert the normal and take its absolute value if thats the case This should not matter for calculations, but we do it anyways
  return Plane{glm::abs(d), normal * (d < 0 ? -1.0f : 1.0f)};
}

/// Input: the three vertices of the triangle
/// Output: if intersects then modify the hit parameter ray.t and return true,
/// otherwise return false
bool intersectRayWithTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, Ray& ray, HitInfo& hitInfo) {
  Plane plane = trianglePlane(v0, v1, v2);

  // We save the old value
  float oldT = ray.t;

  if (!intersectRayWithPlane(plane, ray)) {
    return false;
  }

  // Ray hits the triangle and the triangle is closer than the previous value
  if (pointInTriangle(v0, v1, v2, plane.normal, ray.origin + ray.direction * ray.t) && ray.t <= oldT) {
    // We manually update the hit info normal
    hitInfo.normal = plane.normal;
    return true;
  }

  // Rollback
  ray.t = oldT;

  return false;
}

/// Input: a sphere with the following attributes: sphere.radius, sphere.center
/// Output: if intersects then modify the hit parameter ray.t and return true,
/// otherwise return false
bool intersectRayWithShape(const Sphere& sphere, Ray& ray, HitInfo& hitInfo) {
  float det = std::pow(2 * glm::dot(ray.direction, ray.origin - sphere.center), 2) - 4 * std::pow(glm::length(ray.direction), 2) * (std::pow(glm::length(ray.origin - sphere.center), 2) - std::pow(sphere.radius, 2));

  // No intersection
  if (det < 0) {
    return false;
  }

  float base = -2 * glm::dot(ray.direction, ray.origin - sphere.center);
  float denom = 2 * std::pow(glm::length(ray.direction), 2);
  float t0 = (base - std::sqrt(det)) / denom;
  float t1 = (base + std::sqrt(det)) / denom;

  // Behind origin
  if (t0 < 0 && t1 < 0) {
    return false;
  }

  // Smallest positive
  float newT = t0 < 0 ? t1 : (t1 < 0 ? t0 : std::min(t0, t1));

  // Too far away
  if (ray.t < newT) {
    return false;
  }

  ray.t = newT;
  return true;
}

/// Input: an axis-aligned bounding box with the following parameters: minimum
/// coordinates box.lower and maximum coordinates box.upper Output: if
/// intersects then modify the hit parameter ray.t and return true, otherwise
/// return false
bool intersectRayWithShape(const AxisAlignedBox& box, Ray& ray) {
  // If 1 or 2 direction components are 0 this should still work because the
  // infinities are filtered out on intersection with min/max functions and else
  // tin > tout will not hold
  glm::vec3 a = (box.lower - ray.origin) / ray.direction;
  glm::vec3 b = (box.upper - ray.origin) / ray.direction;
  float tin = std::max({std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)});
  float tout = std::min({std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)});

  // Miss
  if (tin > tout || tout < 0) {
    return false;
  }

  // If tin < 0 then we are inside the box and the first intersection in front
  // of the origin will be the outgoing point
  float newT = tin < 0 ? tout : tin;

  // Too far away
  if (ray.t < newT) {
    return false;
  }

  ray.t = newT;
  return true;
}


//Here we are just doing the phongShading
glm::vec3 phongShading(const Scene& scene, const HitInfo hitInfo, const Ray& ray) {

    glm::vec3 hit = ray.origin + ray.t * ray.direction;

    glm::vec3 normal = glm::normalize(hitInfo.normal);
    glm::vec3 colour(0, 0, 0); 
    for(PointLight lights : scene.pointLights){
        glm::vec3 light = lights.position - hit;
        light = glm::normalize(light);
        
        //calculate the diffuse
        glm::vec3 diffuse = hitInfo.material.kd * glm::max(glm::dot(normal, light), 0.0f) * lights.color;
        

        glm::vec3 view = glm::normalize(ray.origin - hit);
        float dot1 = glm::dot(normal, light);
        float dot2 = glm::dot(normal, view);
        if (dot1 * dot2 <= 0.0f) {
            continue;
        }
        glm::vec3 reflected = glm::reflect(-light, normal);
        float spec = glm::pow(glm::max(glm::dot(view, reflected), 0.0f), hitInfo.material.shininess);
        //calculating the specular
        glm::vec3 specular = hitInfo.material.ks * spec;

        colour += (diffuse + specular);
    }
    
    for (SphericalLight sphericalLight : scene.sphericalLight) {
        glm::vec3 light = sphericalLight.position - hit;
        light = glm::normalize(light);
        glm::vec3 view = glm::normalize(ray.origin - hit);
        float dot1 = glm::dot(normal, light);
        float dot2 = glm::dot(normal, view);
        if (dot1 * dot2 <= 0.0f) {
            continue;
        }

        //calculate the diffuse
        glm::vec3 diffuse = hitInfo.material.kd * glm::max(glm::dot(normal, light), 0.0f) * sphericalLight.color;
        colour += diffuse;

    }
 
    return colour;
}
