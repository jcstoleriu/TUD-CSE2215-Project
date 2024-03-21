#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <stb_image.h>
DISABLE_WARNINGS_POP()
#include <iostream>
#include "image.h"

Image::Image(const std::filesystem::path &filePath) {
    if (!std::filesystem::exists(filePath)) {
        std::cerr << "Texture file " << filePath << " does not exists!" << std::endl;
        throw std::exception();
    }

    const std::string filePathStr = filePath.string(); // Create l-value so c_str() is safe.
    int numChannels;
    stbi_uc* pixels = stbi_load(filePathStr.c_str(), &m_width, &m_height, &numChannels, STBI_rgb);

    if (numChannels < 3) {
        std::cerr << "Only textures with 3 or more color channels are supported. " << filePath << " has " << numChannels << " channels" << std::endl;
        throw std::exception();
    }
    if (!pixels) {
        std::cerr << "Failed to read texture " << filePath << " using stb_image.h" << std::endl;
        throw std::exception();
    }

    std::cout << "Num channels: " << numChannels << std::endl;
    for (int i = 0; i < m_width * m_height * numChannels; i += numChannels) {
        m_pixels.emplace_back(pixels[i + 0] / 255.0F, pixels[i + 1] / 255.0F, pixels[i + 2] / 255.0F);
    }

    stbi_image_free(pixels);
}

glm::vec3 Image::getPixel(const int x, const int y) const {
    // Outside the image
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return glm::vec3(0.0F);
    }

    // TODO: read the correct pixel from m_pixels.
    return glm::vec3(0.0F);
}
