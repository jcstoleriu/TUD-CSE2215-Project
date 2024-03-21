#pragma once

#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <filesystem>
#include <vector>

class Screen {
public:
    Screen(const size_t width, const size_t height);

    void clear(const glm::vec3 &color);

    void setPixel(const size_t x, const size_t y, const glm::vec3 &color);

    void writeBitmapToFile(const std::filesystem::path &filePath);

    void draw();

private:
    size_t width;
    size_t height;
    std::vector<glm::vec3> pixels;
    //GLuint texture;
    uint32_t texture;
};
