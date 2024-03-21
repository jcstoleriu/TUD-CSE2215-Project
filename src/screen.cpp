#include "disable_all_warnings.h"
#include "opengl_includes.h"
DISABLE_WARNINGS_PUSH()
#include <glm/common.hpp>
#include <glm/vec4.hpp>
#include <stb_image_write.h>
DISABLE_WARNINGS_POP()
#include "screen.h"

Screen::Screen(const size_t width, const size_t height): width(width), height(height), pixels(width * height, glm::vec3(0.0F)) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Screen::clear(const glm::vec3 &color) {
    std::fill(std::begin(pixels), std::end(pixels), color);
}

void Screen::setPixel(size_t x, size_t y, const glm::vec3 &color) {
    // Outside the screen
    if (x >= width || y >= height) {
        return;
    }

    // In the window/camera class we use (0, 0) at the bottom left corner of the screen (as used by GLFW).
    // OpenGL / stbi like the origin / (-1,-1) to be at the TOP left corner so transform the y coordinate.
    const size_t i = (height - 1 - y) * width + x;
    pixels[i] = glm::vec4(color, 1.0F);
}

void Screen::writeBitmapToFile(const std::filesystem::path &filePath) {
    std::vector<glm::u8vec4> textureData8Bits(pixels.size());
    std::transform(std::begin(pixels), std::end(pixels), std::begin(textureData8Bits),
        [](const glm::vec3& color) {
            const glm::vec3 clampedColor = glm::clamp(color, 0.0F, 1.0F);
            return glm::u8vec4(glm::vec4(clampedColor, 1.0F) * 255.0F);
        });

    std::string filePathString = filePath.string();
    stbi_write_bmp(filePathString.c_str(), int(width), int(height), 4, textureData8Bits.data());
}

void Screen::draw() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, (GLsizei) width, (GLsizei) height, 0, GL_RGB, GL_FLOAT, pixels.data());

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_NORMALIZE);
    glColor3f(1.0F, 1.0F, 1.0F);

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_QUADS);
    glTexCoord2f(0.0F, 1.0F);
    glVertex3f(-1.0F, -1.0F, 0.0F);
    glTexCoord2f(1.0F, 1.0F);
    glVertex3f(1.0F, -1.0F, 0.0F);
    glTexCoord2f(1.0F, 0.0F);
    glVertex3f(1.0F, 1.0F, 0.0F);
    glTexCoord2f(0.0F, 0.0F);
    glVertex3f(-1.0F, 1.0F, 0.0F);
    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glPopAttrib();
}
