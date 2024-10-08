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


GLuint loadCubemap(const std::vector<std::string>& faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        std::cout << "Loading cubemap texture at " << faces[i] << "\n";
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        assert(data && "Cubemap texture loading failed");
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


std::vector<TextureBindContext::Bind> TextureBindContext::s_Slots;
GLint TextureBindContext::s_MaxTextureSlots;

void TextureBindContext::Init() {
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &s_MaxTextureSlots);
    s_Slots.resize(s_MaxTextureSlots);
    for (int i = 0; i < s_MaxTextureSlots; i++) {
        s_Slots[i].type = 0;
    }
}
void TextureBindContext::ResetAll() {
    for (int i = 0; i < s_MaxTextureSlots; i++) {
        GL_CALL(glActiveTexture(GL_TEXTURE0 + i));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
        GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));

        s_Slots[i].type = 0;
    }
}
void TextureBindContext::Set(int slot, GLenum type, GLuint texture) {
    if (s_Slots[slot].type != 0) {
        std::cout << "[WARNING] Texture slot overwrite\n";
    }
    s_Slots[slot] = { type, texture };
}
void TextureBindContext::Overwrite(int slot, GLenum type, GLuint texture) {
    s_Slots[slot] = { type, texture };
}
void TextureBindContext::ApplyAll() {
    
    for (int i = 0; i < s_MaxTextureSlots; i++) {
        GL_CALL(glActiveTexture(GL_TEXTURE0 + i));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
        GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
        if (s_Slots[i].type != 0) {
            GL_CALL(glBindTexture(s_Slots[i].type, s_Slots[i].texture));
        }
    }
}