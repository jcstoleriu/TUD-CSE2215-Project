#pragma once

#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <filesystem>
#include <vector>

class Image {
public:
    Image(const std::filesystem::path &filePath);

    glm::vec3 getPixel(const int x, const int y) const;

private:
    int m_width;
    int m_height;
    std::vector<glm::vec3> m_pixels;
};
