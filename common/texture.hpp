#pragma once

#include <string_view>

using GLuint = unsigned int;

// Load a .BMP file using our custom loader
GLuint loadBMP_custom(std::string_view imagepath);

//// Since GLFW 3, glfwLoadTexture2D() has been removed. You have to use another
/// texture loading library, / or do it yourself (just like loadBMP_custom and
/// loadDDS) / Load a .TGA file using GLFW's own loader
// GLuint loadTGA_glfw(const char * imagepath);

// Load a .DDS file using GLFW's own loader
GLuint loadDDS(std::string_view imagepath);
