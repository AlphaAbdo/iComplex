#pragma once

#include <iostream>
#include <cstdio>
#include <fstream>
#include <externVars.hpp>


inline int lastx = 0;
inline int lasty = 0;
inline bool leftClicked = false;
inline bool middleClicked = false;
inline bool rightClicked = false;
inline bool haveDoubles = true;

inline std::fstream fileStream;

namespace mainWindow{
    extern void reloadComputeShader();
}
extern void char_callback(GLFWwindow* window, unsigned int codepoint);
extern void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

extern void processInput(GLFWwindow *window);

extern void framebuffer_size_callback(GLFWwindow* window, int width, int height);

extern void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

extern void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

extern void reshapeFunc(GLFWwindow* window, int w, int h);

extern void glfw_error_callback(int error, const char* description);