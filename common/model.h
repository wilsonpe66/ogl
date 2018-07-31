#pragma once

#include "gl_base.h"
#include <vector>

#include <optional>
#include <string_view>

namespace model_ns {

class render_state_type {
  std::optional<int> buffer_index;

  inline void destroy() noexcept;

public:
  render_state_type();
  render_state_type(int buffer_index);
  render_state_type(const render_state_type &) = delete;
  render_state_type(render_state_type &&);

  render_state_type &operator=(const render_state_type &) = delete;
  render_state_type &operator=(render_state_type &&) noexcept;
  ~render_state_type();
};

template <typename T, std::size_t _size = sizeof(T) / sizeof(int)>
class vbo_type {
  std::optional<GLuint> _buffer_id;
  inline void destroy() noexcept {
    if (_buffer_id) {
      glDeleteBuffers(1, &(*_buffer_id));
    }
  }

public:
  vbo_type() : _buffer_id{} {}
  vbo_type(vbo_type &&data) noexcept {
    _buffer_id = data._buffer_id;
    data._buffer_id = std::nullopt;
  }

  vbo_type(const render_state_type &) = delete;
  vbo_type(const std::vector<T> &data) : _buffer_id{} {
    GLuint buffer_id;
    glGenBuffers(1, &buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), &data[0],
                 GL_STATIC_DRAW);
    _buffer_id = buffer_id;
  };

  vbo_type &operator=(const vbo_type &) = delete;
  vbo_type &operator=(vbo_type &&data) noexcept {
    if (this != &data) {
      destroy();
      std::swap(_buffer_id, data._buffer_id);
    }
    return *this;
  }

  ~vbo_type() { destroy(); }

  inline model_ns::render_state_type render(int buffer_index) const {
    glEnableVertexAttribArray(buffer_index);
    glBindBuffer(GL_ARRAY_BUFFER, *_buffer_id);
    glVertexAttribPointer(buffer_index, // attribute
                          _size,        // size
                          GL_FLOAT,     // type
                          GL_FALSE,     // normalized?
                          0,            // stride
                          (void *)0     // array buffer offset
    );
    return {buffer_index};
  }
};

// export class model
class model {
  // Read our .obj file
  std::vector<glm::vec3> vertices;
  vbo_type<glm::vec3> vertexbuffer;

  std::vector<glm::vec2> uvs;
  vbo_type<glm::vec2> uvbuffer;

  std::vector<glm::vec3> normals;
  vbo_type<glm::vec3> normalbuffer;

public:
  model(std::string_view sv);
  model(const model &) = delete;
  model &operator=(const model &) = delete;
  inline const std::vector<glm::vec3> &get_vertices() const { return vertices; }
  inline const std::vector<glm::vec2> &get_uvs() const { return uvs; }
  inline const std::vector<glm::vec3> &get_normals() const { return normals; }
  void render() const noexcept;
};
} // namespace model_ns
