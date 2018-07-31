#pragma once

#include "gl_base.h"
#include <vector>

void indexVBO(const std::vector<glm::vec3> &in_vertices,
              const std::vector<glm::vec2> &in_uvs,
              const std::vector<glm::vec3> &in_normals,
              std::vector<unsigned short> &out_indices,
              std::vector<glm::vec3> &out_vertices,
              std::vector<glm::vec2> &out_uvs,
              std::vector<glm::vec3> &out_normals);

void indexVBO_TBN(const std::vector<glm::vec3> &in_vertices,
                  const std::vector<glm::vec2> &in_uvs,
                  const std::vector<glm::vec3> &in_normals,
                  const std::vector<glm::vec3> &in_tangents,
                  const std::vector<glm::vec3> &in_bitangents,
                  std::vector<unsigned short> &out_indices,
                  std::vector<glm::vec3> &out_vertices,
                  std::vector<glm::vec2> &out_uvs,
                  std::vector<glm::vec3> &out_normals,
                  std::vector<glm::vec3> &out_tangents,
                  std::vector<glm::vec3> &out_bitangents);
