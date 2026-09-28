#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Global constants
extern int windowWidth;
extern int windowHeight;

// Function prototypes
void initOpenGL();
void renderFrame();
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window, float deltaTime);

#endif // WINDOW_H
