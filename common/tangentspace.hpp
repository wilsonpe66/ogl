#pragma once

void computeTangentBasis(const std::vector<glm::vec3> &vertices,
                         const std::vector<glm::vec2> &uvs,
                         const std::vector<glm::vec3> &normals,
                         std::vector<glm::vec3> &tangents,
                         std::vector<glm::vec3> &bitangents);
