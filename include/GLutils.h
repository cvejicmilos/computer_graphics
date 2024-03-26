#pragma once

#include <glad/glad.h>
#include <iostream>
#include <sstream>
#include <vector>

// Helper to crash when an issue arise by dereferencing NULL
#define CRASH(...) (*((volatile int*)0) = 0)


// Helper functions to make the GL_CALL macro
inline const char* glErrorToString(GLenum error) {
    switch (error) {
        case GL_NO_ERROR:           return "GL_NO_ERROR";
        case GL_INVALID_ENUM:       return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:      return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:  return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY:      return "GL_OUT_OF_MEMORY";
        default:                    return "Unknown Error";
    }
}
inline void glCheckError(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        std::stringstream ss;
        ss << "[OpenGL Error] (" << glErrorToString(error) << "): " 
           << function << " " << file << ":" << line;
        std::cerr << ss.str() << std::endl;
        CRASH();
        
    }
}
inline void glClearError() {
    while (glGetError() != GL_NO_ERROR);
}

inline void checkProgram(GLuint program) {
    GLint validationStatus;
    // Skip validation in Release mode so we can run
    // faster.
#ifdef _DEBUG
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &validationStatus);
#else
    validationStatus = GL_TRUE;
#endif
    if (validationStatus == GL_FALSE) {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        GLchar *logMessage = new GLchar[logLength];
        glGetProgramInfoLog(program, logLength, NULL, logMessage);
        std::cout << "Program Runtime Validation Error: [" << validationStatus << "] " << logMessage << "\n";
        delete[] logMessage;
        CRASH();
    }
}

#ifdef _DEBUG
// For every OpenGL call, clear the error, make the call, and check the error
#define GL_CALL(x) glClearError();\
    x;\
    glCheckError(#x, __FILE__, __LINE__);
#else
    #define GL_CALL(x) x
#endif


struct Texture {
    GLuint id = 0;
    int width, height, channels;
};
Texture loadTexture(const std::string& path);
void deleteTexture(const Texture& texture);

GLuint loadCubemap(const std::vector<std::string>& faces);

// Helper manager to avoid coflict in texture binding
class TextureBindContext {
private:
    struct Bind {
        GLenum type;
        GLuint texture;
    };
    static std::vector<Bind> s_Slots;
    static GLint s_MaxTextureSlots;
public:
    static void Init();

    // Zero binds o all slots
    static void ResetAll();
    // Sets slot to be applied
    static void Set(int slot, GLenum type, GLuint texture);
    // Same as set but shows intention of overwriting
    // so no warning message
    static void Overwrite(int slot, GLenum type, GLuint texture);
    // Apply slots in OpenGL
    static void ApplyAll();
};