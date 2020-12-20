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


bool pointInTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& n, const glm::vec3& p)
{
    //total area of triangle
    glm::vec3 crossProduct = glm::cross((v1 - v0), (v2 - v0));
    float length = glm::length(crossProduct);
    float area = length / 2;

    //area of small triangle 1
    glm::vec3 crossProduct1 = glm::cross((v2 - v1), (p - v1));
    float u = glm::length(crossProduct1) / 2;

    //area of small triangle 2
    glm::vec3 crossProduct2 = glm::cross((v0 - v2), (p - v2));
    float v = glm::length(crossProduct2) / 2;

    //area of small triangle 3
    glm::vec3 crossProduct3 = glm::cross((v1 - v0), (p - v0));
    float w = glm::length(crossProduct3) / 2;

    //if area of 3 triangles is equal to total area then point is inside, else it is outside
    bool result = glm::abs(area - u - v - w) < 1e-6;


    return result;
}

bool intersectRayWithPlane(const Plane& plane, Ray& ray)
{
    float denominator = glm::dot(ray.direction, plane.normal);

    //checking if ray and plane are parallel
    if (glm::abs(denominator) < 1e-6) {
        return false;
    }

    float numerator = plane.D - glm::dot(ray.origin, plane.normal);

    float t = numerator / denominator;

    //only positive values of t as we want to ray to be in front
    if (t >= 0) {
        if (t < ray.t) {
            ray.t = t;
            return true;
        }
    }
    return false;
}

Plane trianglePlane(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
{
    Plane plane;

    plane.normal = glm::cross((v0 - v2), (v1 - v2)) / glm::length(glm::cross((v0 - v2), (v1 - v2)));
    plane.D = glm::dot(plane.normal, v0);

    return plane;
}

/// Input: the three vertices of the triangle
/// Output: if intersects then modify the hit parameter ray.t and return true, otherwise return false
bool intersectRayWithTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, Ray& ray, HitInfo& hitInfo)
{
    //calculate the plane containing the triangle
    Plane plane = trianglePlane(v0, v1, v2);

    float temp = ray.t;

    //calculate the intersection point of the ray and the plane
    bool intersection = intersectRayWithPlane(plane, ray);
    if (intersection == true) {
        glm::vec3 p = ray.origin + ray.direction * ray.t;
        //check if the point is inside the triangle
        bool result = pointInTriangle(v0, v1, v2, plane.normal, p);

        if (temp < ray.t || glm::length(temp * ray.direction) < glm::length(ray.t * ray.direction)) {
            ray.t = temp;
        }
        if (result == true) {
            hitInfo.normal = plane.normal;
            return true;
        }
        else {
            //roll back the ray.t
            ray.t = temp;
            return false;
        }
    }
    return false;
}



/// Input: a sphere with the following attributes: sphere.radius, sphere.center
/// Output: if intersects then modify the hit parameter ray.t and return true, otherwise return false
bool intersectRayWithShape(const Sphere& sphere, Ray& ray, HitInfo& hitInfo)
{
    glm::vec3 temp = ray.origin;
    ray.origin = ray.origin - sphere.center;
    float A = (ray.direction.x * ray.direction.x) + (ray.direction.y * ray.direction.y) + (ray.direction.z * ray.direction.z);
    float B = 2 * ((ray.direction.x * ray.origin.x) + (ray.direction.y * ray.origin.y) + (ray.direction.z * ray.origin.z));
    float C = (ray.origin.x * ray.origin.x) + (ray.origin.y * ray.origin.y) + (ray.origin.z * ray.origin.z) - (sphere.radius * sphere.radius);


    float discriminant = sqrt((B * B) - (4 * A * C));

    float t = 0.0f;

    //the different possibilities of discriminants
    if (discriminant > 0) {
        float t0 = (-B + discriminant) / (2 * A);
        float t1 = (-B - discriminant) / (2 * A);
        if (t0 < 0) {
            t0 = 100000.0f;
        }
        if (t1 < 0) {
            t1 = 100000.0f;
        }
        t = glm::min(t0, t1);
    }
    else if (discriminant == 0) {
        t = -B / (2 * A);
    }
    else {
        ray.origin = temp;
        return false;
    }

    ray.origin = temp;
    float temp2 = ray.t;
    ray.t = t;
    if (temp2 < ray.t || glm::length(temp2 * ray.direction) < glm::length(ray.t * ray.direction)) {
        ray.t = temp2;
    }
    glm::vec3 p = ray.origin + ray.t * ray.direction;
    hitInfo.normal = glm::normalize(p - sphere.center);
    hitInfo.material = sphere.material;
    return true;
}

/// Input: an axis-aligned bounding box with the following parameters: minimum coordinates box.lower and maximum coordinates box.upper
/// Output: if intersects then modify the hit parameter ray.t and return true, otherwise return false
bool intersectRayWithShape(const AxisAlignedBox& box, Ray& ray)
{
    float txmin = (box.lower.x - ray.origin.x) / ray.direction.x;
    float txmax = (box.upper.x - ray.origin.x) / ray.direction.x;

    float tymin = (box.lower.y - ray.origin.y) / ray.direction.y;
    float tymax = (box.upper.y - ray.origin.y) / ray.direction.y;

    float tzmin = (box.lower.z - ray.origin.z) / ray.direction.z;
    float tzmax = (box.upper.z - ray.origin.z) / ray.direction.z;

    float tinx = glm::min(txmin, txmax);
    float toutx = glm::max(txmin, txmax);

    float tiny = glm::min(tymin, tymax);
    float touty = glm::max(tymin, tymax);

    float tinz = glm::min(tzmin, tzmax);
    float toutz = glm::max(tzmin, tzmax);

    float tin = std::max({ tinx, tiny,tinz });
    float tout = std::min({ toutx, touty, toutz });

    //check validity of tin
    if (tin > tout || tout < 0) {
        return false;
    }

    //check if the ray is intersecting inside the cube as well
    if (tin < 0) {
        ray.t = tout;
        return true;
    }


    float temp = ray.t;
    ray.t = tin;

    if (temp < ray.t || glm::length(temp * ray.direction) < glm::length(ray.t * ray.direction)) {
        ray.t = temp;
    }

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
