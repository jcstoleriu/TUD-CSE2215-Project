#include "scene.h"

Scene loadScene(SceneType type, const std::filesystem::path &dataDir) {
	Scene scene;
	switch (type) {
		case CornellBox: {
			// Load a 3D model of a Dragon
			std::vector<Mesh> subMeshes = loadMesh(dataDir / "CornellBox-Mirror-Rotated.obj", true);
			std::move(std::begin(subMeshes), std::end(subMeshes), std::back_inserter(scene.meshes));
			scene.pointLights.push_back(PointLight{glm::vec3(0.0F, 0.58F, 0.0F), glm::vec3(1.0F)}); // Light at the top of the box
			break;
		}
	};

	return scene;
}
