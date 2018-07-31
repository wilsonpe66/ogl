#pragma once
#include <string_view>

using GLuint = unsigned int;

GLuint LoadShaders(std::string_view vertex_file_path,
                   std::string_view fragment_file_path);
