#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

// Loads an image file and returns an OpenGL texture ID
GLuint loadTexture(const char* filepath);

#endif // TEXTURE_H
