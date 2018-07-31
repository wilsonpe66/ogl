#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "shader.hpp"

namespace {
enum class gl_vertex_mode_type : unsigned int {
  vertext_shader = GL_VERTEX_SHADER,
  fragment_shader = GL_FRAGMENT_SHADER
};

GLuint load_shader(std::string_view file_path,
                   gl_vertex_mode_type gl_vertex_mode) noexcept {
  // Read the Vertex Shader code from the file
  std::string shader_code;
  std::ifstream shader_stream(file_path.data(), std::ios::in);
  if (shader_stream.is_open()) {
    std::stringstream sstr;
    sstr << shader_stream.rdbuf();
    shader_code = sstr.str();
    shader_stream.close();
  } else {
    std::cerr << "Impossible to open " << file_path
              << ". Are you in the right directory ? Don't forget to read the "
                 "FAQ !\n";
    getchar();
    return 0;
  }

  // Create the shaders
  const GLuint shader_id =
      glCreateShader(static_cast<unsigned int>(gl_vertex_mode));

  // Compile Shader
  std::cout << "Compiling shader : " << file_path << "\n";
  char const *VertexSourcePointer = shader_code.c_str();
  glShaderSource(shader_id, 1, &VertexSourcePointer, nullptr);
  glCompileShader(shader_id);

  // Check Vertex Shader
  GLint result = GL_FALSE;
  int info_log_length;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
  glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
  if (info_log_length > 0) {
    std::vector<char> VertexShaderErrorMessage(info_log_length + 1);
    glGetShaderInfoLog(shader_id, info_log_length, nullptr,
                       &VertexShaderErrorMessage[0]);
    std::cerr << &VertexShaderErrorMessage[0] << '\n';
  }

  return shader_id;
}
} // namespace

GLuint LoadShaders(std::string_view vertex_file_path,
                   std::string_view fragment_file_path) {

  const GLuint vertex_shader_id =
      load_shader(vertex_file_path, gl_vertex_mode_type::vertext_shader);
  const GLuint fragment_shader_id =
      load_shader(fragment_file_path, gl_vertex_mode_type::fragment_shader);

  // Link the program
  std::cout << "Linking program\n";
  const GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, vertex_shader_id);
  glAttachShader(ProgramID, fragment_shader_id);
  glLinkProgram(ProgramID);

  // Check the program
  GLint result = GL_FALSE;
  int info_log_length;
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &info_log_length);
  if (info_log_length > 0) {
    std::vector<char> ProgramErrorMessage(info_log_length + 1);
    glGetProgramInfoLog(ProgramID, info_log_length, nullptr,
                        &ProgramErrorMessage[0]);
    std::cerr << &ProgramErrorMessage[0] << '\n';
  }

  glDetachShader(ProgramID, vertex_shader_id);
  glDetachShader(ProgramID, fragment_shader_id);

  glDeleteShader(vertex_shader_id);
  glDeleteShader(fragment_shader_id);

  return ProgramID;
}
