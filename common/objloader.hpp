#pragma once
#include <glm/glm.hpp>
#include <string_view>
#include <vector>

bool loadOBJ(std::string_view path, std::vector<glm::vec3> &out_vertices,
             std::vector<glm::vec2> &out_uvs,
             std::vector<glm::vec3> &out_normals);

bool loadAssImp(std::string_view path, std::vector<short unsigned int> &indices,
                std::vector<glm::vec3> &vertices, std::vector<glm::vec2> &uvs,
                std::vector<glm::vec3> &normals);
