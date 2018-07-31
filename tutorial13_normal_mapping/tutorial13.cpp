// Include standard headers
#include <iostream>
#include <vector>

#include <common/gl_base.h>
GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/controls.hpp>
#include <common/model.h>
#include <common/shader.hpp>
#include <common/tangentspace.hpp>
#include <common/texture.hpp>
#include <common/vboindexer.hpp>

int main(void) {
  // Initialise GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    getchar();
    return -1;
  }

  glfwWindowHint(GLFW_SAMPLES, 1);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
                 GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); // So that
  // glBegin/glVertex/glEnd work

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(1024, 768, "Tutorial 13 - Normal Mapping", nullptr,
                            nullptr);
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

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  // Hide the mouse and enable unlimited mouvement
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
  const GLuint programID =
      LoadShaders("NormalMapping.vertexshader", "NormalMapping.fragmentshader");

  // Get a handle for our "MVP" uniform
  const GLuint MatrixID = glGetUniformLocation(programID, "MVP");
  const GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
  const GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
  const GLuint ModelView3x3MatrixID = glGetUniformLocation(programID, "MV3x3");

  // Load the texture
  const GLuint DiffuseTexture = loadDDS("diffuse.DDS");
  const GLuint NormalTexture = loadBMP_custom("normal.bmp");
  const GLuint SpecularTexture = loadDDS("specular.DDS");

  // Get a handle for our "myTextureSampler" uniform
  const GLuint DiffuseTextureID =
      glGetUniformLocation(programID, "DiffuseTextureSampler");
  const GLuint NormalTextureID =
      glGetUniformLocation(programID, "NormalTextureSampler");
  const GLuint SpecularTextureID =
      glGetUniformLocation(programID, "SpecularTextureSampler");

  // Read our .obj file
  const model_ns::model cylinder("cylinder.obj");

  std::vector<glm::vec3> tangents;
  std::vector<glm::vec3> bitangents;
  computeTangentBasis(cylinder.get_vertices(), cylinder.get_uvs(),
                      cylinder.get_normals(), tangents, bitangents // output
  );

  std::vector<unsigned short> indices;
  std::vector<glm::vec3> indexed_vertices;
  std::vector<glm::vec2> indexed_uvs;
  std::vector<glm::vec3> indexed_normals;
  std::vector<glm::vec3> indexed_tangents;
  std::vector<glm::vec3> indexed_bitangents;
  indexVBO_TBN(cylinder.get_vertices(), cylinder.get_uvs(),
               cylinder.get_normals(), tangents, bitangents, indices,
               indexed_vertices, indexed_uvs, indexed_normals, indexed_tangents,
               indexed_bitangents);

  // Load it into a VBO

  const model_ns::vbo_type<glm::vec3> tangentbuffer{indexed_tangents};
  const model_ns::vbo_type<glm::vec3> bitangentbuffer{indexed_bitangents};

  // Generate a buffer for the indices as well
  const GLuint elementbuffer = [indices]() {
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
    if (currentTime - lastTime >= 1.0) {
      // If last prinf() was more than 1sec ago cout and reset
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
    const glm::mat4 ModelViewMatrix = ViewMatrix * ModelMatrix;
    const glm::mat3 ModelView3x3Matrix = glm::mat3(ModelViewMatrix);
    const glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
    glUniformMatrix3fv(ModelView3x3MatrixID, 1, GL_FALSE,
                       &ModelView3x3Matrix[0][0]);

    glm::vec3 lightPos = glm::vec3(0, 0, 4);
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

    // Bind our diffuse texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, DiffuseTexture);
    // Set our "DiffuseTextureSampler" sampler to use Texture Unit 0
    glUniform1i(DiffuseTextureID, 0);

    // Bind our normal texture in Texture Unit 1
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, NormalTexture);
    // Set our "NormalTextureSampler" sampler to use Texture Unit 1
    glUniform1i(NormalTextureID, 1);

    // Bind our specular texture in Texture Unit 2
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, SpecularTexture);
    // Set our "SpecularTextureSampler" sampler to use Texture Unit 2
    glUniform1i(SpecularTextureID, 2);

    {
      // 4th attribute buffer : tangents
      const model_ns::render_state_type tangentbuffer_render_state =
          tangentbuffer.render(3);

      // 5th attribute buffer : bitangents
      const model_ns::render_state_type bitangentbuffer_render_state =
          bitangentbuffer.render(4);

      cylinder.render();

      // Index buffer
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

      // Draw the triangles !
      glDrawElements(GL_TRIANGLES,      // mode
                     indices.size(),    // count
                     GL_UNSIGNED_SHORT, // type
                     (void *)0          // element array buffer offset
      );
    }
    ////////////////////////////////////////////////////////
    // DEBUG ONLY !!!
    // Don't use this in real code !!
    ////////////////////////////////////////////////////////

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((const GLfloat *)&ProjectionMatrix[0]);
    glMatrixMode(GL_MODELVIEW);
    glm::mat4 MV = ViewMatrix * ModelMatrix;
    glLoadMatrixf((const GLfloat *)&MV[0]);

    glUseProgram(0);

    // normals
    glColor3f(0, 0, 1);
    glBegin(GL_LINES);
    for (unsigned int i = 0; i < indices.size(); i++) {
      glm::vec3 p = indexed_vertices[indices[i]];
      glVertex3fv(&p.x);
      glm::vec3 o = glm::normalize(indexed_normals[indices[i]]);
      p += o * 0.1f;
      glVertex3fv(&p.x);
    }
    glEnd();
    // tangents
    glColor3f(1, 0, 0);
    glBegin(GL_LINES);
    for (unsigned int i = 0; i < indices.size(); i++) {
      glm::vec3 p = indexed_vertices[indices[i]];
      glVertex3fv(&p.x);
      glm::vec3 o = glm::normalize(indexed_tangents[indices[i]]);
      p += o * 0.1f;
      glVertex3fv(&p.x);
    }
    glEnd();
    // bitangents
    glColor3f(0, 1, 0);
    glBegin(GL_LINES);
    for (unsigned int i = 0; i < indices.size(); i++) {
      glm::vec3 p = indexed_vertices[indices[i]];
      glVertex3fv(&p.x);
      glm::vec3 o = glm::normalize(indexed_bitangents[indices[i]]);
      p += o * 0.1f;
      glVertex3fv(&p.x);
    }
    glEnd();
    // light pos
    glColor3f(1, 1, 1);
    glBegin(GL_LINES);
    glVertex3fv(&lightPos.x);
    lightPos += glm::vec3(1, 0, 0) * 0.1f;
    glVertex3fv(&lightPos.x);
    lightPos -= glm::vec3(1, 0, 0) * 0.1f;
    glVertex3fv(&lightPos.x);
    lightPos += glm::vec3(0, 1, 0) * 0.1f;
    glVertex3fv(&lightPos.x);
    glEnd();

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();

  } // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         glfwWindowShouldClose(window) == 0);

  // Cleanup VBO and shader
  glDeleteBuffers(1, &elementbuffer);
  glDeleteProgram(programID);
  glDeleteTextures(1, &DiffuseTexture);
  glDeleteTextures(1, &NormalTexture);
  glDeleteTextures(1, &SpecularTexture);
  glDeleteVertexArrays(1, &VertexArrayID);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}
