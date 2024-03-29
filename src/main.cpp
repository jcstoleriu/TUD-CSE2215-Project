#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
DISABLE_WARNINGS_POP()
#include <array>
#include <atomic>
#include <deque>
#include <iostream>
#include <random>
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

// This is the main application. The code in here does not need to be modified.
static constexpr const size_t WIDTH = 800;
static constexpr const size_t HEIGHT = 800;
static const std::filesystem::path dataPath{ DATA_DIR };
static const std::filesystem::path outputPath{ OUTPUT_DIR };

static void setOpenGLMatrices(const Trackball &camera);
static void renderOpenGL(const Scene &scene, const Trackball &camera, int selectedLight);

// This is the main rendering function. You are free to change this function in any way (including the function signature).
static void renderRayTracing(const Scene& scene, const Trackball& camera, const BoundingVolumeHierarchy& bvh, const ShadingData &data, std::default_random_engine rng, Screen& screen) {
    std::atomic_size_t render_progress = 0;
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
            glm::vec3 color = get_color(camera.position(), scene, bvh, data, rng, cameraRay);
            screen.setPixel(x, y, color);

            size_t i = ++render_progress;
            if (i % 64 == 0) {
                float f = 100.0F * i / (WIDTH * HEIGHT);
                std::cout << "\r\033[2KProgress: " << f << "%" << std::flush;
            }
        }
    }
    std::cout << std::endl;
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

    Scene scene = loadScene(sceneType, dataPath);

    std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
    BoundingVolumeHierarchy bvh{&scene};
    std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
    std::cout << "Time to compute bounding volume hierarchy: " << std::chrono::duration<float, std::milli>(end - start).count() << " millisecond(s)" << std::endl;

    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng;
    rng.seed(seed);
    std::cout << "Seed: " << seed << std::endl;

    std::optional<Ray> optDebugRay;
    int bvhDebugLevel = 0;
    bool debugBVH{ false };
    int selectedLight = 0;
    bool showSelectedMesh = false;
    bool showSelectedMeshE = false; // for showing meshes selected for edit
    int selectedMesh = 0;
    int selectedMeshA = 0;
    int selectedMeshB = 0;
    float offset = 0.0F;
    float scalar = 1.0F;
    // matrix of (scale, offset), fill with 0
    std::vector<std::vector<std::vector<float>>> transforms;
    for (int i = 0; i < scene.meshes.size(); i++) {
        std::vector<std::vector<float>> row;
        for (int j = 0; j < scene.meshes.size(); j++) {
            std::vector<float> elem;
            elem.push_back(scalar);
            elem.push_back(offset);
            row.push_back(elem);
        }
        transforms.push_back(row);
    }
        
    ShadingData data = ShadingData{false, 3, 32, transforms};

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

    while (!window.shouldClose()) {
        window.updateInput();

        // === Setup the UI ===
        ImGui::Begin("Menu");
        ImGui::SliderInt("Depth", &data.max_traces, 1, 8);
        ImGui::SliderInt("Samples", &data.samples, 0, 64);
        {
            if (ImGui::InputScalar("Seed", ImGuiDataType_::ImGuiDataType_U32, (void *) &seed, NULL, NULL, "%u", 0)) {
                rng.seed(seed);
            }
        }
        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::Button("Render to file")) {
            {
                const std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
                renderRayTracing(scene, camera, bvh, data, rng, screen);
                const std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
                std::cout << "Time to render image: " << std::chrono::duration<float, std::milli>(end - start).count() / 1000.0F << " second(s)" << std::endl;
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
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Meshes");
        if (!scene.meshes.empty()) {
            {
                std::vector<std::string> options;
                for (size_t i = 0; i < scene.meshes.size(); i++) {
                    options.push_back("Mesh " + std::to_string(i + 1));
                }

                std::vector<const char *> optionsPointers;
                std::transform(std::begin(options), std::end(options), std::back_inserter(optionsPointers), [](const auto &str) { return str.c_str(); });
                ImGui::Combo("Selected mesh", &selectedMesh, optionsPointers.data(), static_cast<int>(optionsPointers.size()));
            }
            {
                Material &material = scene.meshes[selectedMesh].material;
                ImGui::ColorEdit3("kd", glm::value_ptr(material.kd));
                ImGui::ColorEdit3("ks", glm::value_ptr(material.ks));
                ImGui::SliderFloat("shininess", &material.shininess, 0.0F, 100.0F);
                // Not supported
                //ImGui::SliderFloat("transparency", &material.transparency, 0.0F, 1.0F);
            }
            {
                ImGui::Checkbox("Highlight mesh", &showSelectedMesh);
            }
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Edits");
        if (!scene.meshes.empty()) {
            {
                std::vector<std::string> optionsA;
                for (size_t i = 0; i < scene.meshes.size(); i++) {
                    optionsA.push_back("Mesh " + std::to_string(i + 1));
                }

                std::vector<std::string> optionsB;
                for (size_t i = 0; i < scene.meshes.size(); i++) {
                    optionsB.push_back("Mesh " + std::to_string(i + 1));
                }

                std::vector<const char*> optionsPointersA;
                std::transform(std::begin(optionsA), std::end(optionsA), std::back_inserter(optionsPointersA), [](const auto& str) { return str.c_str(); });
                ImGui::Combo("Selected mesh A", &selectedMeshA, optionsPointersA.data(), static_cast<int>(optionsPointersA.size()));
                std::vector<const char*> optionsPointersB;
                std::transform(std::begin(optionsB), std::end(optionsB), std::back_inserter(optionsPointersB), [](const auto& str) { return str.c_str(); });
                ImGui::Combo("Selected mesh B", &selectedMeshB, optionsPointersB.data(), static_cast<int>(optionsPointersB.size()));
            }
            {
                bool changedOffset = ImGui::SliderFloat("offset", &offset, 0.0F, 100.0F);
                bool changedScale = ImGui::SliderFloat("scalar value", &scalar, 0.0F, 10.0F);
                if (changedScale) {
                    updateTransforms(data, scalar, 0, selectedMeshA, selectedMeshB);
                }
                if (changedOffset) {
                    updateTransforms(data, offset, 1, selectedMeshA, selectedMeshB);
                }
            }
            {
                ImGui::Checkbox("Highlight selected meshes (red for A and green for B)", &showSelectedMeshE);
            }
        }

        // Clear screen.
        glClearDepth(1.0F);
        glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        renderOpenGL(scene, camera, selectedLight);
        if (optDebugRay) {
            data.debug = true;
            // We do not reuse the original rng to make the debug output consistent
            std::default_random_engine rand;
            rand.seed(seed);
            (void) get_color(camera.position(), scene, bvh, data, rand, *optDebugRay);
            data.debug = false;
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

        if (!scene.meshes.empty() && showSelectedMesh) {
            glPushAttrib(GL_ALL_ATTRIB_BITS);
            setOpenGLMatrices(camera);
            glDisable(GL_LIGHTING);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            drawAABB(AxisAlignedBox{scene.meshes[selectedMesh].lower, scene.meshes[selectedMesh].upper}, DrawMode::WIREFRAME, glm::vec3(0.0F, 1.0F, 1.0F), 1.0F);
            glPopAttrib();
        }

        if (!scene.meshes.empty() && showSelectedMeshE) {
            glPushAttrib(GL_ALL_ATTRIB_BITS);
            setOpenGLMatrices(camera);
            glDisable(GL_LIGHTING);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            drawAABB(AxisAlignedBox{ scene.meshes[selectedMeshA].lower, scene.meshes[selectedMeshA].upper }, DrawMode::WIREFRAME, glm::vec3(1.0F, 0.0F, 0.0F), 1.0F);
            drawAABB(AxisAlignedBox{ scene.meshes[selectedMeshB].lower, scene.meshes[selectedMeshB].upper }, DrawMode::WIREFRAME, glm::vec3(0.0F, 1.0F, 0.0F), 1.0F);
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
