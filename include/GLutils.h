#pragma once

#include <glad/glad.h>
#include <iostream>
#include <sstream>

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

        
    }
}
inline void glClearError() {
    while (glGetError() != GL_NO_ERROR);
}

// For every OpenGL call, clear the error, make the call, and check the error
#define GL_CALL(x) glClearError();\
    x;\
    glCheckError(#x, __FILE__, __LINE__);



struct Texture {
    GLuint id = 0;
    int width, height, channels;
};
Texture loadTexture(const std::string& path);
void deleteTexture(const Texture& texture);