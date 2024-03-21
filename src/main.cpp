#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
DISABLE_WARNINGS_POP()
#include <array>
#include <iostream>
#include <string>
#include "bounding_volume_hierarchy.h"
#include "draw.h"
#include "illumination.h"
#include "screen.h"
#include "trackball.h"
#include "window.h"
#ifdef USE_OPENMP
#include <omp.h>
#endif

#define REFLECTION_MAX_TRACES (1 << 5)

// This is the main application. The code in here does not need to be modified.
static constexpr const size_t WIDTH = 800;
static constexpr const size_t HEIGHT = 800;
static const std::filesystem::path dataPath{ DATA_DIR };
static const std::filesystem::path outputPath{ OUTPUT_DIR };

// NOTE(Mathijs): separate function to make recursion easier (could also be done with lambda + std::function).
static glm::vec3 getFinalColor(const glm::vec3 &camera, const Scene &scene, const BoundingVolumeHierarchy &bvh, Ray ray, size_t depth) {
  HitInfo hitInfo;

  // Ray miss
  if (depth >= REFLECTION_MAX_TRACES || !bvh.intersect(ray, hitInfo)) {
      // Draw a red debug ray if the ray missed.
      drawRay(ray, glm::vec3(1.0F, 0.0F, 0.0F));

      // Set the color of the pixel to black if the ray misses.
      return glm::vec3(0.0F);
  }


  glm::vec3 color = shader(scene, bvh, ray, hitInfo, camera);

  // Draw a white debug ray.
  drawRay(ray, glm::vec3(1.0F));

  // If Ks is not black (glm::vec3{0, 0, 0} has magnitude 0)
  if (glm::length(hitInfo.material.ks) > 0) {
    glm::vec3 normal = glm::normalize(hitInfo.normal);

    // Because we intersect we know t != infinity
    glm::vec3 hitPoint = ray.origin + ray.direction * ray.t;

    // Reflection of ray direction over the given normal
    // I don't know if the result is normalized so we do it manually anyways
    glm::vec3 reflectionDir = glm::normalize(ray.direction - 2 * glm::dot(ray.direction, normal) * normal);

    // We give the reflection ray an offset of 0.01 * direction, otherwise we start inside of the triangle we hit
    // This may not be optimal
    Ray reflRay = Ray{hitPoint + reflectionDir * 0.01F, reflectionDir};

    glm::vec3 reflecColor = getFinalColor(camera, scene, bvh, reflRay, depth + 1);

    color += reflecColor;
  }

  return glm::clamp(color, 0.0F, 1.0F);
}

static inline glm::vec3 getFinalColor(const glm::vec3 &camera, const Scene &scene, const BoundingVolumeHierarchy &bvh, Ray ray) {
  return getFinalColor(camera, scene, bvh, ray, 0);
}

static void setOpenGLMatrices(const Trackball &camera);
static void renderOpenGL(const Scene& scene, const Trackball& camera, int selectedLight);

static void sampleViewpoints(const Scene& scene) {
    for (const Mesh& mesh : scene.meshes) {
        //do stuff
    }
}

// This is the main rendering function. You are free to change this function in any way (including the function signature).
static void renderRayTracing(const Scene& scene, const Trackball& camera, const BoundingVolumeHierarchy& bvh, Screen& screen) {
    sampleViewpoints(scene);
#ifdef USE_OPENMP
#pragma omp parallel for
#endif   
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            // NOTE: (-1, -1) at the bottom left of the screen, (+1, +1) at the top right of the screen.
            glm::vec2 normalizedPixelPos{
                float(x) / WIDTH * 2.0F - 1.0F,
                float(y) / HEIGHT * 2.0F - 1.0F
            };
            const Ray cameraRay = camera.generateRay(normalizedPixelPos);
            screen.setPixel(x, y, getFinalColor(camera.position(), scene, bvh, cameraRay));
        }
    }
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    Trackball::printHelp();
    std::cout << "Press the [R] key on your keyboard to create a ray towards the mouse cursor" << std::endl;

    Window window{"Paper Summary Demo", glm::ivec2(WIDTH, HEIGHT), OpenGLVersion::GL2};
    Screen screen{WIDTH, HEIGHT};
    Trackball camera{&window, glm::radians(50.0F), 3.0F};
    camera.setCamera(glm::vec3(0.0F), glm::radians(glm::vec3(20.0F, 20.0F, 0.0F)), 3.0F);

    SceneType sceneType{SceneType::CornellBox};
    std::optional<Ray> optDebugRay;
    Scene scene = loadScene(sceneType, dataPath);
    const std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
    BoundingVolumeHierarchy bvh{&scene};
    const std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
    std::cout << "Time to compute bounding volume hierarchy: " << std::chrono::duration<float, std::milli>(end - start).count() << " millisecond(s)" << std::endl;

    int bvhDebugLevel = 0;
    bool debugBVH{ false };

    window.registerKeyCallback([&](int key, int scancode, int action, int mods) {
            (void) scancode;
            (void) mods;

            if (action != GLFW_PRESS) {
                return;
            }

            switch (key) {
                case GLFW_KEY_R: {
                    // Shoot a ray. Produce a ray from camera to the far plane.
                    const glm::vec2 tmp = window.getNormalizedCursorPos();
                    optDebugRay = camera.generateRay(tmp * 2.0F - 1.0F);
                    break;
                }
                case GLFW_KEY_ESCAPE: {
                    window.close();
                    break;
                }
            }
        });

    int selectedLight = 0;

    while (!window.shouldClose()) {
        window.updateInput();

        // === Setup the UI ===
        ImGui::Begin("Menu");
        {
            constexpr std::array items{"Cornell Box (with mirror)", ""};
            if (ImGui::Combo("Scenes", reinterpret_cast<int*>(&sceneType), items.data(), int(items.size()))) {
                optDebugRay.reset();
                scene = loadScene(sceneType, dataPath);
                bvh = BoundingVolumeHierarchy(&scene);
                if (optDebugRay) {
                    HitInfo dummy{};
                    bvh.intersect(*optDebugRay, dummy);
                }
            }
        }
        if (ImGui::Button("Render to file")) {
            {
                const std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
                renderRayTracing(scene, camera, bvh, screen);
                const std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
                std::cout << "Time to render image: " << std::chrono::duration<float, std::milli>(end - start).count() << " millisecond(s)" << std::endl;
            }
            screen.writeBitmapToFile(outputPath / "render.bmp");
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Debugging");
        ImGui::Checkbox("Draw BVH", &debugBVH);
        if (debugBVH) {
            ImGui::SliderInt("BVH Level", &bvhDebugLevel, 0, bvh.numLevels() - 1);
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Lights");
        if (!scene.pointLights.empty()) {
            {
                std::vector<std::string> options;
                for (size_t i = 0; i < scene.pointLights.size(); i++) {
                    options.push_back("Point Light " + std::to_string(i + 1));
                }

                std::vector<const char*> optionsPointers;
                std::transform(std::begin(options), std::end(options), std::back_inserter(optionsPointers), [](const auto& str) { return str.c_str(); });
                ImGui::Combo("Selected light", &selectedLight, optionsPointers.data(), static_cast<int>(optionsPointers.size()));
            }
            {
                ImGui::DragFloat3("Light position", glm::value_ptr(scene.pointLights[selectedLight].position), 0.01F, -3.0F, 3.0F);
                ImGui::ColorEdit3("Light color", glm::value_ptr(scene.pointLights[selectedLight].color));
            }
        }

        // Clear screen.
        glClearDepth(1.0F);
        glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        renderOpenGL(scene, camera, selectedLight);
        if (optDebugRay) {
            // Call getFinalColor for the debug ray. Ignore the result but tell the function that it should
            // draw the rays instead.
            enableDrawRay = true;
            (void) getFinalColor(camera.position(), scene, bvh, *optDebugRay);
            enableDrawRay = false;
        }
        glPopAttrib();

        if (debugBVH) {
            glPushAttrib(GL_ALL_ATTRIB_BITS);
            setOpenGLMatrices(camera);
            glDisable(GL_LIGHTING);
            glEnable(GL_DEPTH_TEST);

            // Enable alpha blending. More info at:
            // https://learnopengl.com/Advanced-OpenGL/Blending
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            bvh.debugDraw(bvhDebugLevel);
            glPopAttrib();
        }

        ImGui::End();

        window.swapBuffers();
    }

    return 0; // execution never reaches this point
}

static void setOpenGLMatrices(const Trackball& camera)
{
    // Load view matrix.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    const glm::mat4 viewMatrix = camera.viewMatrix();
    glMultMatrixf(glm::value_ptr(viewMatrix));

    // Load projection matrix.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const glm::mat4 projectionMatrix = camera.projectionMatrix();
    glMultMatrixf(glm::value_ptr(projectionMatrix));
}

static void renderOpenGL(const Scene& scene, const Trackball& camera, int selectedLight) {
    // Normals will be normalized in the graphics pipeline.
    glEnable(GL_NORMALIZE);
    // Activate rendering modes.
    glEnable(GL_DEPTH_TEST);
    // Draw front and back facing triangles filled.
    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_FILL);
    // Interpolate vertex colors over the triangles.
    glShadeModel(GL_SMOOTH);
    setOpenGLMatrices(camera);

    glDisable(GL_LIGHTING);
    // Render point lights as very small dots
    for (const PointLight &light : scene.pointLights) {
        drawSphere(light.position, 0.01F, light.color);
    }

    if (!scene.pointLights.empty()) {
        // Draw a big yellow sphere and then the small light sphere on top.
        const PointLight &light = scene.pointLights[selectedLight];
        drawSphere(light.position, 0.05F, glm::vec3(1.0F, 1.0F, 0.0F));
        glDisable(GL_DEPTH_TEST);
        drawSphere(light.position, 0.01F, light.color);
        glEnable(GL_DEPTH_TEST);
    }

    // Activate the light in the legacy OpenGL mode.
    glEnable(GL_LIGHTING);

    for (size_t i = 0; i < scene.pointLights.size(); i++) {
        const PointLight &light = scene.pointLights[i];
        glEnable((GLenum) GL_LIGHT0 + i);
        const glm::vec4 position4{light.position, 1};
        glLightfv((GLenum) GL_LIGHT0 + i, GL_POSITION, glm::value_ptr(position4));
        const glm::vec4 color4{glm::clamp(light.color, 0.0F, 1.0F), 1.0F};
        const glm::vec4 zero4{0.0F, 0.0F, 0.0F, 1.0F};
        glLightfv((GLenum) GL_LIGHT0 + i, GL_AMBIENT, glm::value_ptr(zero4));
        glLightfv((GLenum) GL_LIGHT0 + i, GL_DIFFUSE, glm::value_ptr(color4));
        glLightfv((GLenum) GL_LIGHT0 + i, GL_SPECULAR, glm::value_ptr(zero4));
        // NOTE: quadratic attenuation doesn't work like you think it would in legacy OpenGL.
        // The distance is not in world space but in NDC space!
        glLightf((GLenum) GL_LIGHT0 + i, GL_CONSTANT_ATTENUATION, 1.0F);
        glLightf((GLenum) GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, 0.0F);
        glLightf((GLenum) GL_LIGHT0 + i, GL_QUADRATIC_ATTENUATION, 0.0F);
    }

    // Draw the scene and the ray (if any).
    drawScene(scene);

    // Draw a colored sphere at the location at which the trackball is looking/rotating around.
    glDisable(GL_LIGHTING);
    drawSphere(camera.lookAt(), 0.01F, glm::vec3(0.2F, 0.2F, 1.0F));
}
