// Include standard headers
#include <iostream>
#include <vector>

#include <common/gl_base.h>
#include <common/model.h>
GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/vboindexer.hpp>

// The ARB_debug_output extension, which is used in this tutorial as an example,
// can call a function of ours with error messages.
// This function must have this precise prototype ( parameters and return value
// ) See http://www.opengl.org/registry/specs/ARB/debug_output.txt , "New Types"
// :
//	The callback function that applications can define, and
//	is accepted by DebugMessageCallbackARB, is defined as:
//
//	    typedef void (APIENTRY *DEBUGPROCARB)(enum source,
//	                                          enum type,
//	                                          uint id,
//	                                          enum severity,
//	                                          sizei length,
//	                                          const char* message,
//	                                          void* userParam);
void APIENTRY DebugOutputCallback(GLenum source, GLenum type, GLuint id,
                                  GLenum severity, GLsizei length,
                                  const GLchar *message,
                                  const void *userParam) {

  std::cout << "OpenGL Debug Output message : ";

  const std::string_view mapped_source = [&source]() {
    switch (source) {
    case GL_DEBUG_SOURCE_API_ARB:
      return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
      return "WINDOW_SYSTEM";
    case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
      return "THIRD_PARTY";
    case GL_DEBUG_SOURCE_APPLICATION_ARB:
      return "APPLICATION";
    }
    return "OTHER";
  }();

  std::cout << "Source : " << mapped_source << "; ";

  const std::string_view mapped_type = [&type]() {
    switch (type) {
    case GL_DEBUG_TYPE_ERROR_ARB:
      return "ERROR";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
      return "DEPRECATED_BEHAVIOR";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
      return "UNDEFINED_BEHAVIOR";
    case GL_DEBUG_TYPE_PORTABILITY_ARB:
      return "PORTABILITY";
    case GL_DEBUG_TYPE_PERFORMANCE_ARB:
      return "PERFORMANCE";
    }
    return "OTHER";
  }();

  std::cout << "Type : " << mapped_type << "; ";

  const std::string_view mapped_severity = [&severity]() {
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH_ARB:
      return "HIGH";
    case GL_DEBUG_SEVERITY_MEDIUM_ARB:
      return "MEDIUM";
    case GL_DEBUG_SEVERITY_LOW_ARB:
      return "LOW";
    }
    return "OTHER";
  }();

  std::cout << "Severity : " << mapped_severity << "; ";

  // You can set a breakpoint here ! Your debugger will stop the program,
  // and the callstack will immediately show you the offending call.
  std::cout << "Message : " << message << "\n";
}

int main(void) {
  // Initialise GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    getchar();
    return -1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
                 GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // ARB_debug_output is a bit special,
  // it requires creating the OpenGL context
  // with paticular flags.
  // GLFW exposes it this way; if you use SDL, SFML, freeGLUT
  // or other, check the documentation.
  // If you use custom code, read the spec :
  // http://www.opengl.org/registry/specs/ARB/debug_output.txt
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

  // Open a window and create its OpenGL context
  window =
      glfwCreateWindow(1024, 768, "Tutorial 12 - Extensions", nullptr, nullptr);
  if (window == nullptr) {
    std::cerr
        << "Failed to open GLFW window. If you have an Intel GPU, they are "
           "not 3.3 compatible. Try the 2.1 version of the tutorials.\n";
    getchar();
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW\n";
    getchar();
    glfwTerminate();
    return -1;
  }

  // Example 1 :
  if (GLEW_AMD_seamless_cubemap_per_texture) {
    std::cout
        << "The GL_AMD_seamless_cubemap_per_texture is present, (but we're not "
           "goint to use it)\n";
    // Now it's legal to call glTexParameterf with the
    // TEXTURE_CUBE_MAP_SEAMLESS_ARB parameter You HAVE to test this, because
    // obviously, this code would fail on non-AMD hardware.
  }

  // Example 2 :
  if (GLEW_ARB_debug_output) {
    std::cout
        << "The OpenGL implementation provides debug output. Let's use it !\n";
    glDebugMessageCallbackARB(&DebugOutputCallback, nullptr);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
  } else {
    std::cout
        << "ARB_debug_output unavailable. You have to use glGetError() and/or "
           "gDebugger to catch mistakes.\n";
  }

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  // Hide the mouse and enable unlimited mouvement
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Set the mouse at the center of the screen
  glfwPollEvents();
  glfwSetCursorPos(window, 1024 / 2, 768 / 2);

  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

  // // Enable depth test
  // glEnable(
  //     GL_DEPTH); // SHOULD BE GL_DEPTH_TEST ! WILL TRIGGER AN ERROR MESSAGE !
  // // Accept fragment if it closer to the camera than the former one
  // glDepthFunc(GL_LEFT); // SHOULD BE GL_LESS ! WILL TRIGGER AN ERROR MESSAGE
  // !
  //
  // // Cull triangles which normal is not towards the camera
  // glEnable(GL_CULL_FACE_MODE); // SHOULD BE GL_CULL_FACE ! WILL TRIGGER AN
  // ERROR
  //                              // MESSAGE !

  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);

  // Cull triangles which normal is not towards the camera
  glEnable(GL_CULL_FACE);

  const GLuint VertexArrayID = []() {
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    return VertexArrayID;
  }();

  // Create and compile our GLSL program from the shaders
  const GLuint programID =
      LoadShaders("StandardShading.vertexshader",
                  "StandardShading_WithSyntaxErrors.fragmentshader");

  // Get a handle for our "MVP" uniform
  const GLuint MatrixID = glGetUniformLocation(programID, "MVP");
  const GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
  const GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

  // Load the texture
  const GLuint Texture = loadDDS("uvmap.DDS");

  // Get a handle for our "myTextureSampler" uniform
  const GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

  // Read our .obj file
  const model_ns::model my_model("suzanne.obj");

  std::vector<unsigned short> indices;
  std::vector<glm::vec3> indexed_vertices;
  std::vector<glm::vec2> indexed_uvs;
  std::vector<glm::vec3> indexed_normals;
  indexVBO(my_model.get_vertices(), my_model.get_uvs(), my_model.get_normals(),
           indices, indexed_vertices, indexed_uvs, indexed_normals);

  // Generate a buffer for the indices as well
  const GLuint elementbuffer = [&indices]() {
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned short), &indices[0],
                 GL_STATIC_DRAW);
    return elementbuffer;
  }();

  // Get a handle for our "LightPosition" uniform
  glUseProgram(programID);
  const GLuint LightID =
      glGetUniformLocation(programID, "LightPosition_worldspace");

  // For speed computation
  double lastTime = glfwGetTime();
  int nbFrames = 0;

  do {

    // Measure speed
    const double currentTime = glfwGetTime();
    nbFrames++;
    if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec
                                         // ago printf and reset
      std::cout << (1000.0 / double(nbFrames)) << " ms/frame\n";
      nbFrames = 0;
      lastTime += 1.0;
    }

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(programID);

    // Compute the MVP matrix from keyboard and mouse input
    computeMatricesFromInputs();
    const glm::mat4 ProjectionMatrix = getProjectionMatrix();
    const glm::mat4 ViewMatrix = getViewMatrix();
    const glm::mat4 ModelMatrix = glm::mat4(1.0);
    const glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

    const glm::vec3 lightPos = glm::vec3(4, 4, 4);
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture);
    // Set our "myTextureSampler" sampler to use Texture Unit 0
    glUniform1i(TextureID, 0);

    my_model.render();

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

    // Draw the triangles !
    glDrawElements(GL_TRIANGLES,      // mode
                   indices.size(),    // count
                   GL_UNSIGNED_SHORT, // type
                   (void *)0          // element array buffer offset
    );

    // Swap buffers
    glfwPollEvents();
    glfwSwapBuffers(window);

  } // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         glfwWindowShouldClose(window) == 0);

  // Cleanup VBO and shader
  glDeleteBuffers(1, &elementbuffer);
  glDeleteProgram(programID);
  glDeleteTextures(1, &Texture);
  glDeleteVertexArrays(1, &VertexArrayID);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}
