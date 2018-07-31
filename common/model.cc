#include "model.h"
#include "objloader.hpp"

#include <array>
#include <type_traits>
template <typename... T, std::size_t n = sizeof...(T)>
auto make_array(T &&... t) {
  return std::array<std::common_type_t<T...>, n>{std::forward<T>(t)...};
}

namespace model_ns {

void render_state_type::destroy() noexcept {
  if (buffer_index) {
    glDisableVertexAttribArray(*buffer_index);
  }
}

render_state_type::render_state_type() : buffer_index{} {}
render_state_type::render_state_type(int buffer_index)
    : buffer_index{buffer_index} {}
render_state_type::render_state_type(render_state_type &&render_state)
    : buffer_index{render_state.buffer_index} {
  render_state.buffer_index = std::nullopt;
}

render_state_type &render_state_type::
operator=(render_state_type &&render_state) noexcept {
  if (this != &render_state) {
    destroy();
    buffer_index = render_state.buffer_index;
    render_state.buffer_index = std::nullopt;
  }
  return *this;
}

render_state_type::~render_state_type() { destroy(); }

model::model(std::string_view sv) {
  const bool res = loadOBJ(sv.data(), vertices, uvs, normals);

  // Load it into a VBO
  vertexbuffer = std::move(vbo_type{vertices});
  uvbuffer = std::move(vbo_type{uvs});
  normalbuffer = std::move(vbo_type{normals});
}
void model::render() const noexcept {
  int buffer_index = 0;
  const auto render_states_vertexbuffer = vertexbuffer.render(buffer_index++);
  const auto render_states_uvbuffer = uvbuffer.render(buffer_index++);
  const auto render_states_normalbuffer = normalbuffer.render(buffer_index);

  // Draw the triangles !
  glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}

} // namespace model_ns
