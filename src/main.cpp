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
#include "sparseMatrix.h"
#ifdef USE_OPENMP
#include <omp.h>
#endif

// This is the main application. The code in here does not need to be modified.
static constexpr const size_t WIDTH = 2000;
static constexpr const size_t HEIGHT = 1400;
static const std::filesystem::path dataPath{ DATA_DIR };
static const std::filesystem::path outputPath{ OUTPUT_DIR };

static void setOpenGLMatrices(const Trackball &camera);
static void renderOpenGL(const Scene &scene, const Trackball &camera, int selectedLight);

// This is the main rendering function. You are free to change this function in any way (including the function signature).
static void renderRayTracing(const Scene& scene, const Trackball& camera, const BoundingVolumeHierarchy& bvh, const ShadingData &data, std::default_random_engine rng, Screen& screen, TransportMatrix &transportMatrix) {
    std::atomic_size_t render_progress = 0;
    // size_t numMeshes = scene.meshes.size();
    // TransportMatrix transportMatrix = TransportMatrix(numMeshes);
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
            Ray cameraRay = camera.generateRay(normalizedPixelPos);
            
            glm::vec3 color = get_color(camera.position(), scene, bvh, data, rng, cameraRay, transportMatrix);
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

static void showArray(std::vector< std::tuple<glm::vec3, glm::vec3>> arr, ImDrawList* drawList) {
    for (const auto& [scalar, offset] : arr) {
        ImGui::BeginGroup();

        ImGui::Text("%.2f\n%.2f\n%.2f", scalar.x, scalar.y, scalar.z);
        ImGui::SameLine();
        ImGui::Text("%.2f\n%.2f\n%.2f", offset.x, offset.y, offset.z);

        ImGui::EndGroup();
        ImGui::SameLine();

        ImU32 white = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0, 1.0, 1.0, 1.0));
        // for some reason operators are not overloaded for ImVec
        ImVec2 start = ImVec2(ImGui::GetItemRectMin().x-5.0, ImGui::GetItemRectMin().y-5.0);
        ImVec2 end = ImVec2(ImGui::GetItemRectMax().x+5.0, ImGui::GetItemRectMax().y+5.0);
        drawList->AddRect(start, end, white);
    }
    ImGui::Dummy(ImVec2(0.0f, 20.0f));
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
    bool showT {false};
    int selectedLight = 0;
    bool showSelectedMesh = false;
    bool showSelectedMeshE = false; // for showing meshes selected for edit
    int selectedMesh = 0;
    int selectedMeshA = 0;
    int selectedMeshB = 0;

    size_t meshCount = scene.meshes.size();
    //std::vector<std::vector<std::tuple<glm::vec3, glm::vec3>>> transforms(meshCount, std::vector<std::tuple<glm::vec3, glm::vec3>>(meshCount, std::tuple(glm::vec3(1.0F), glm::vec3(0.0F))));
    SparseMatrix transforms = SparseMatrix(meshCount, meshCount);
    ShadingData data = ShadingData{false, 3, 32, &transforms};

    TransportMatrix transportMatrix = TransportMatrix(meshCount, meshCount);
    std::cout << transportMatrix.matrix.size() << " " << transportMatrix.matrix[0].size() << std::endl;

    // to do the haar transform you need a vector with 2^k elements, which would take a bit to set up
    // so i think using pre-defined vectors is fine for example purposes
    std::tuple<glm::vec3, glm::vec3> a1 = std::tuple(glm::vec3(8.0), glm::vec3(8.0));
    std::tuple<glm::vec3, glm::vec3> a2 = std::tuple(glm::vec3(4.0), glm::vec3(4.0));
    std::tuple<glm::vec3, glm::vec3> a3 = std::tuple(glm::vec3(1.0), glm::vec3(1.0));
    std::tuple<glm::vec3, glm::vec3> a4 = std::tuple(glm::vec3(3.0), glm::vec3(3.0));
    std::tuple<glm::vec3, glm::vec3> a5 = std::tuple(glm::vec3(5.0), glm::vec3(5.0));
    std::tuple<glm::vec3, glm::vec3> a6 = std::tuple(glm::vec3(2.0), glm::vec3(2.0));
    std::tuple<glm::vec3, glm::vec3> a7 = std::tuple(glm::vec3(2.0), glm::vec3(2.0));
    std::tuple<glm::vec3, glm::vec3> a8 = std::tuple(glm::vec3(1.0), glm::vec3(1.0));
    std::vector< std::tuple<glm::vec3, glm::vec3>> arr;
    arr.push_back(a1);
    arr.push_back(a2);
    arr.push_back(a3);
    arr.push_back(a4);
    arr.push_back(a5);
    arr.push_back(a6);
    arr.push_back(a7);
    arr.push_back(a8);

    static int level = 1;
    std::vector< std::tuple<glm::vec3, glm::vec3>> transf = haarTransformRow(arr, level);

    std::vector< std::tuple<glm::vec3, glm::vec3>> reconstr = haarInvTransformRow(transf, level);

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
        ImGui::SliderInt("Samples", &data.samples, 0, 128);
        {
            if (ImGui::InputScalar("Seed", ImGuiDataType_::ImGuiDataType_U32, (void *) &seed, NULL, NULL, "%u", 0)) {
                rng.seed(seed);
            }
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::Button("Render to file")) {
            {
                const std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
                renderRayTracing(scene, camera, bvh, data, rng, screen, transportMatrix);
                const std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
                std::cout << "Time to render image: " << std::chrono::duration<float, std::milli>(end - start).count() / 1000.0F << " second(s)" << std::endl;
            }
            screen.writeBitmapToFile(outputPath / "render.bmp");
        }
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
        ImGui::Text("Transforms (A -> B)");
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
                ImGui::Combo("Mesh A", &selectedMeshA, optionsPointersA.data(), static_cast<int>(optionsPointersA.size()));
                std::vector<const char*> optionsPointersB;
                std::transform(std::begin(optionsB), std::end(optionsB), std::back_inserter(optionsPointersB), [](const auto& str) { return str.c_str(); });
                ImGui::Combo("Mesh B", &selectedMeshB, optionsPointersB.data(), static_cast<int>(optionsPointersB.size()));
            }
            {
                auto [scalar, offset] = (*data.transforms).get(selectedMeshA, selectedMeshB);
                if (ImGui::InputFloat3("Scalar", glm::value_ptr(scalar)) || ImGui::InputFloat3("Offset", glm::value_ptr(offset))) {
                    (*data.transforms).set(selectedMeshA, selectedMeshB, std::tuple(scalar, offset));
                }
            }
            ImGui::Checkbox("Highlight selected meshes (red for A and green for B)", &showSelectedMeshE);
        }
        ImGui::End();
        // Additional ImGui window for transport matrix visualization
        ImGui::SetNextWindowPos(ImVec2(80.0, 10.0) , ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(800.0, 900.0), ImGuiCond_FirstUseEver);
        ImGui::Begin("Transport Matrix Visualization");
        // TransportMatrix transportMatrix;
        ImGui::Checkbox("Show T Matrix", &showT);
        if (showT) {
            if (!scene.meshes.empty() && selectedMeshA < scene.meshes.size() && selectedMeshB < scene.meshes.size()) {
                ImGui::Text("Transport Matrix:");
                ImGui::Text("%d x %d", transportMatrix.matrix.size(), transportMatrix.matrix[0].size());
                const int x = transportMatrix.matrix.size();
                const int y = transportMatrix.matrix[0].size();
                float temp_max = 1.0f;
                for (int i = 0; i < x; ++i) {
                    for (int j = 0; j < y; ++j) {
                        float current = transportMatrix.matrix[i][j].x;
                        if (current >= temp_max) {
                            temp_max = current;
                        }
                    }
                }
                //ImGui::BeginChild("Matrix", ImVec2(500, 200), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
                ImDrawList* drawList = ImGui::GetWindowDrawList();

                ImGui::Text("Logical representation: ");
                for (int i = 0; i < x; ++i) {
                    ImGui::BeginGroup();
                    for (int j = 0; j < y; ++j) {  
                        ImGui::BeginGroup();
                        const glm::vec3 color = glm::vec3(transportMatrix.matrix[i][j].x / temp_max);
                        glm::vec3 normalizedColor = color;
                        auto [scalar, offset] = (*data.transforms).get(i, j);
                        normalizedColor = normalizedColor * scalar + offset;

                        ImGui::Text("%.2f\n%.2f\n%.2f", scalar.x, scalar.y, scalar.z);
                        ImGui::SameLine();
                        ImGui::Text("%.2f\n%.2f\n%.2f", offset.x, offset.y, offset.z);
                        ImGui::SameLine();
                        ImVec4 imguiColor(normalizedColor.r, normalizedColor.g, normalizedColor.b, 1.0f);  // Convert glm::vec3 to ImVec4
                        ImGui::ColorButton("##color", imguiColor, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoAlpha, ImVec2(20, 20));
                        ImGui::EndGroup();

                        if (j < y - 1) ImGui::SameLine();
                        // highlight selected element
                        if (i == selectedMeshA && j == selectedMeshB) {
                            ImU32 yellow = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0, 1.0, 0.0, 1.0));
                            drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), yellow);
                        }
                    }
                    ImGui::EndGroup();
                    // highlight selected row
                    if (i == selectedMeshA) {
                        ImU32 yellow = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0, 1.0, 0.0, 1.0));
                        drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), yellow);
                    }
                }
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Text("Representation in memory: ");
                if ((*data.transforms).getSize() == 0)
                    ImGui::Text("No non-default elements defined.");
                else {
                    for (const auto& [key, val] : (*data.transforms).getMatrix()) {
                        auto &[scalar, offset] = val;
                        ImGui::Text("(%d, %d) -> ((%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f))", std::get<0>(key), std::get<1>(key),
                            scalar.x, scalar.y, scalar.z, offset.x, offset.y, offset.z);
                    }
                }
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Text("Haar transform example: ");

                static int level = std::log2(arr.size());
                if (ImGui::SliderInt("Transform level", &level, 1, std::log2(arr.size()))) {
                    transf = haarTransformRow(arr, level);
                    reconstr = haarInvTransformRow(transf, level);
                }
                ImGui::Dummy(ImVec2(10.0f, 20.0f));

                ImGui::Text("Original array: ");
                showArray(arr, drawList);

                // apply Haar transform and show projected array
                ImGui::Dummy(ImVec2(10.0f, 20.0f));
                ImGui::Text("Haar wavelet projection: ");
                showArray(transf, drawList);

                if (ImGui::Button("Cull small coefficients")) {
                    for (auto& [scalar, offset] : transf) {
                        // cull if between -1 and 1 and reconstruct original array again to show error
                        // in our case all elements are the same, but that is obviously not always the case
                        if (scalar.x > -1.0 && scalar.x < 1.0) {
                            scalar = glm::vec3(0.0);
                            offset = glm::vec3(0.0);
                            reconstr = haarInvTransformRow(transf, level);
                        }
                    }
                }

                ImGui::Dummy(ImVec2(10.0f, 20.0f));
                ImGui::Text("Reconstructed: ");
                showArray(reconstr, drawList);
            }
        }
        ImGui::End();

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
            size_t numMeshes = scene.meshes.size();
            TransportMatrix transportMatrix = TransportMatrix(numMeshes);
            (void) get_color(camera.position(), scene, bvh, data, rand, *optDebugRay, transportMatrix);
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

        // ImGui::End();

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
