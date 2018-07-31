// Include standard headers
#include <iostream>
#include <vector>

#include <common/gl_base.h>
GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <common/controls.hpp>
#include <common/model.h>
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/vboindexer.hpp>

int main() {
  // Initialise GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW"
                 "\n";
    getchar();
    return -1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
                 GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(1024, 768, "Tutorial 08 - Basic Shading", nullptr,
                            nullptr);
  if (window == nullptr) {
    std::cerr
        << "Failed to open GLFW window. If you have an Intel GPU, they are "
           "not 3.3 compatible. Try the 2.1 version of the tutorials."
           "\n";
    getchar();
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW"
                 "\n";
    getchar();
    glfwTerminate();
    return -1;
  }

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  // Hide the mouse and enable unlimited mouvement
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  // Set the mouse at the center of the screen
  glfwPollEvents();
  glfwSetCursorPos(window, 1024 / 2, 768 / 2);

  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

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
  const GLuint programID = LoadShaders("StandardShading.vertexshader",
                                       "StandardShading.fragmentshader");

  // Get a handle for our "MVP" uniform
  const GLuint MatrixID = glGetUniformLocation(programID, "MVP");
  const GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
  const GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

  // Load the texture
  const GLuint Texture = loadDDS("uvmap.DDS");

  // Get a handle for our "myTextureSampler" uniform
  const GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

  const model_ns::model my_model("suzanne.obj");

  // Get a handle for our "LightPosition" uniform
  glUseProgram(programID);
  const GLuint LightID =
      glGetUniformLocation(programID, "LightPosition_worldspace");

  do {

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
