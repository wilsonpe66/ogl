#include "objloader.hpp"
#include <fstream>
#include <iostream>
#include <limits>
#include <string.h>
#include <vector>

// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide :
// - Binary files. Reading a model should be just a few memcpy's away, not
//   parsing a file at runtime.In short    : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

bool loadOBJ(std::string_view path, std::vector<glm::vec3> &out_vertices,
             std::vector<glm::vec2> &out_uvs,
             std::vector<glm::vec3> &out_normals) {
  std::cout << "Loading OBJ file " << path << "...\n";

  std::vector<glm::vec3> temp_vertices;
  std::vector<glm::vec2> temp_uvs;
  std::vector<glm::vec3> temp_normals;

  std::ifstream in(path.data());
  if (!in) {
    std::cerr
        << "Impossible to open the file ! Are you in the right path ? See "
           "Tutorial 1 for details\n";
    getchar();
    return false;
  }

  for (;;) {

    char lineHeader[128];
    // read the first word of the line
    in >> lineHeader;
    if (in.eof()) {
      break; // EOF = End Of File. Quit the loop.
    }

    // else : parse lineHeader
    if (strcmp(lineHeader, "v") == 0) {
      glm::vec3 vertex;
      in >> vertex.x >> vertex.y >> vertex.z;
      temp_vertices.emplace_back(vertex);
    } else if (strcmp(lineHeader, "vt") == 0) {
      glm::vec2 uv;
      in >> uv.x >> uv.y;
      uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture,
                    // which are inverted. Remove if you want to use TGA or BMP
                    // loaders.
      temp_uvs.emplace_back(uv);
    } else if (strcmp(lineHeader, "vn") == 0) {
      glm::vec3 normal;
      in >> normal.x >> normal.y >> normal.z;
      temp_normals.emplace_back(normal);
    } else if (strcmp(lineHeader, "f") == 0) {
      for (int i = 0; i < 3; ++i) {
        char dummy;
        unsigned int vertexIndex, uvIndex, normalIndex;
        in >> vertexIndex >> dummy >> uvIndex >> dummy >> normalIndex;

        out_vertices.emplace_back(temp_vertices.at(vertexIndex - 1));
        out_uvs.emplace_back(temp_uvs.at(uvIndex - 1));
        out_normals.emplace_back(temp_normals.at(normalIndex - 1));
      }
    }
    in.ignore(std::numeric_limits<std::size_t>::max(), '\n');
  }

  in.close();
  return true;
}

#ifdef USE_ASSIMP // don't use this #define, it's only for me (it AssImp fails
                  // to compile on your machine, at least all the other
                  // tutorials still work)

// Include AssImp
#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/scene.h>       // Output data structure

bool loadAssImp(std::string_view path, std::vector<short unsigned int> &indices,
                std::vector<glm::vec3> &vertices, std::vector<glm::vec2> &uvs,
                std::vector<glm::vec3> &normals) {

  Assimp::Importer importer;

  const aiScene *scene = importer.ReadFile(
      path.data(),
      0 /*aiProcess_JoinIdenticalVertices | aiProcess_SortByPType*/);
  if (!scene) {
    std::cerr << importer.GetErrorString() << '\n';
    getchar();
    return false;
  }
  const aiMesh *mesh =
      scene->mMeshes[0]; // In this simple example code we always use the 1rst
                         // mesh (in OBJ files there is often only one anyway)

  // Fill vertices positions
  vertices.reserve(mesh->mNumVertices);
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    const aiVector3D pos = mesh->mVertices[i];
    vertices.emplace_back(glm::vec3{pos.x, pos.y, pos.z});
  }

  // Fill vertices texture coordinates
  uvs.reserve(mesh->mNumVertices);
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    const aiVector3D UVW =
        mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp
                                    // supports 8 UV sets.
    uvs.emplace_back(glm::vec2{UVW.x, UVW.y});
  }

  // Fill vertices normals
  normals.reserve(mesh->mNumVertices);
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    const aiVector3D n = mesh->mNormals[i];
    normals.emplace_back(glm::vec3{n.x, n.y, n.z});
  }

  // Fill face indices
  indices.reserve(3 * mesh->mNumFaces);
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    const auto &face = mesh->mFaces[i];
    // Assume the model has only triangles.
    indices.push_back(face.mIndices[0]);
    indices.push_back(face.mIndices[1]);
    indices.push_back(face.mIndices[2]);
  }

  // The "scene" pointer will be deleted automatically by "importer"
  return true;
}

#endif // USE_ASSIMP
