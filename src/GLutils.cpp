#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "GLutils.h"

Texture loadTexture(const std::string& path) {
    Texture texture;

    void* data = stbi_load(path.c_str(), &texture.width, &texture.height, &texture.channels, 4);

    assert(data && "Texture loading failed");

    GL_CALL(glGenTextures(1, &texture.id));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, texture.id));

    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));

    GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));

    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

    texture.channels = 4;

    stbi_image_free(data);

    return texture;
}
void deleteTexture(const Texture& texture) {
    GL_CALL(glBindTexture(GL_TEXTURE_2D, texture.id));
    GL_CALL(glDeleteTextures(1, &texture.id));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}