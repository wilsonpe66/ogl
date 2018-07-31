// Include standard headers
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <common/gl_base.h>
#include <common/model.h>
GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>

int main(void) {
  // Initialise GLFW
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
                 GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(1024, 768, "Tutorial 05 - Textured Cube", nullptr,
                            nullptr);
  if (window == nullptr) {
    fprintf(stderr,
            "Failed to open GLFW window. If you have an Intel GPU, they are "
            "not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    return -1;
  }

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);

  const GLuint VertexArrayID = []() {
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    return VertexArrayID;
  }();

  // Create and compile our GLSL program from the shaders
  const GLuint programID = LoadShaders("TransformVertexShader.vertexshader",
                                       "TextureFragmentShader.fragmentshader");

  // Get a handle for our "MVP" uniform
  const GLuint MatrixID = glGetUniformLocation(programID, "MVP");

  // Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit
  // <-> 100 units
  const glm::mat4 Projection =
      glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
  // Camera matrix
  const glm::mat4 View = glm::lookAt(
      glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
      glm::vec3(0, 0, 0), // and looks at the origin
      glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
  );
  // Model matrix : an identity matrix (model will be at the origin)
  const glm::mat4 Model = glm::mat4(1.0f);
  // Our ModelViewProjection : multiplication of our 3 matrices
  const glm::mat4 MVP =
      Projection * View *
      Model; // Remember, matrix multiplication is the other way around

  // Load the texture using any two methods
  // GLuint Texture = loadBMP_custom("uvtemplate.bmp");
  const GLuint Texture = loadDDS("uvtemplate.DDS");

  // Get a handle for our "myTextureSampler" uniform
  const GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

  // Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive
  // vertices give a triangle. A cube has 6 faces with 2 triangles each, so this
  // makes 6*2=12 triangles, and 12*3 vertices
  static const std::vector<glm::vec3> vertices = {
      {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f},  {-1.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, -1.0f},   {-1.0f, -1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},
      {1.0f, -1.0f, 1.0f},   {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},
      {1.0f, 1.0f, -1.0f},   {1.0f, -1.0f, -1.0f},  {-1.0f, -1.0f, -1.0f},
      {-1.0f, -1.0f, -1.0f}, {-1.0f, 1.0f, 1.0f},   {-1.0f, 1.0f, -1.0f},
      {1.0f, -1.0f, 1.0f},   {-1.0f, -1.0f, 1.0f},  {-1.0f, -1.0f, -1.0f},
      {-1.0f, 1.0f, 1.0f},   {-1.0f, -1.0f, 1.0f},  {1.0f, -1.0f, 1.0f},
      {1.0f, 1.0f, 1.0f},    {1.0f, -1.0f, -1.0f},  {1.0f, 1.0f, -1.0f},
      {1.0f, -1.0f, -1.0f},  {1.0f, 1.0f, 1.0f},    {1.0f, -1.0f, 1.0f},
      {1.0f, 1.0f, 1.0f},    {1.0f, 1.0f, -1.0f},   {-1.0f, 1.0f, -1.0f},
      {1.0f, 1.0f, 1.0f},    {-1.0f, 1.0f, -1.0f},  {-1.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, 1.0f},    {-1.0f, 1.0f, 1.0f},   {1.0f, -1.0f, 1.0f}};

  // Two UV coordinatesfor each vertex. They were created with Blender.
  static const std::vector<glm::vec2> uvs = {
      {0.000059f, 0.000004f}, {0.000103f, 0.336048f}, {0.335973f, 0.335903f},
      {1.000023f, 0.000013f}, {0.667979f, 0.335851f}, {0.999958f, 0.336064f},
      {0.667979f, 0.335851f}, {0.336024f, 0.671877f}, {0.667969f, 0.671889f},
      {1.000023f, 0.000013f}, {0.668104f, 0.000013f}, {0.667979f, 0.335851f},
      {0.000059f, 0.000004f}, {0.335973f, 0.335903f}, {0.336098f, 0.000071f},
      {0.667979f, 0.335851f}, {0.335973f, 0.335903f}, {0.336024f, 0.671877f},
      {1.000004f, 0.671847f}, {0.999958f, 0.336064f}, {0.667979f, 0.335851f},
      {0.668104f, 0.000013f}, {0.335973f, 0.335903f}, {0.667979f, 0.335851f},
      {0.335973f, 0.335903f}, {0.668104f, 0.000013f}, {0.336098f, 0.000071f},
      {0.000103f, 0.336048f}, {0.000004f, 0.671870f}, {0.336024f, 0.671877f},
      {0.000103f, 0.336048f}, {0.336024f, 0.671877f}, {0.335973f, 0.335903f},
      {0.667969f, 0.671889f}, {1.000004f, 0.671847f}, {0.667979f, 0.335851f}};

  const model_ns::vbo_type<glm::vec3> vertexbuffer{vertices};
  const model_ns::vbo_type<glm::vec2> uvbuffer{uvs};

  do {

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(programID);

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture);
    // Set our "myTextureSampler" sampler to use Texture Unit 0
    glUniform1i(TextureID, 0);

    // 1rst attribute buffer : vertices
    const model_ns::render_state_type render_states_vertexbuffer =
        vertexbuffer.render(0);
    // 2nd attribute buffer : UVs
    const model_ns::render_state_type render_states_uvbuffer =
        uvbuffer.render(1);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0,
                 12 * 3); // 12*3 indices starting at 0 -> 12 triangles

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();

  } // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         glfwWindowShouldClose(window) == 0);

  // Cleanup VBO and shader
  glDeleteProgram(programID);
  glDeleteTextures(1, &Texture);
  glDeleteVertexArrays(1, &VertexArrayID);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}
