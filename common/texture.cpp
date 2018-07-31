#include <fstream>
#include <iostream>
#include <memory>
#include <string.h>
#include <string_view>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

GLuint loadBMP_custom(std::string_view imagepath) {
  std::cout << "Reading image " << imagepath << '\n';

  // Open the file
  std::ifstream image_file(imagepath.data(), std::ios::binary | std::ios::in);
  if (!image_file) {
    std::cerr << imagepath
              << " could not be opened. Are you in the right directory ? Don't "
                 "forget to read the FAQ !\n";
    getchar();
    return 0;
  }

  char header[54];
  // Read the header, i.e. the 54 first bytes

  // If less than 54 bytes are read, problem
  if (image_file.read(header, 54)) {
    std::cerr << "Not a correct BMP file\n";
    return 0;
  }
  // A BMP files always begins with "BM"
  if (header[0] != 'B' || header[1] != 'M') {
    std::cerr << "Not a correct BMP file\n";
    return 0;
  }
  // Make sure this is a 24bpp file
  if (*(int *)&(header[0x1E]) != 0) {
    std::cerr << "Not a correct BMP file\n";
    return 0;
  }
  if (*(int *)&(header[0x1C]) != 24) {
    std::cerr << "Not a correct BMP file\n";
    return 0;
  }

  // Read the information about the image
  const unsigned int width = *(int *)&(header[0x12]);
  const unsigned int height = *(int *)&(header[0x16]);
  const unsigned int imageSize = [&width, &height](int image_size) {
    // Some BMP files are misformatted, guess missing information
    // 3 : one byte for each Red, Green and Blue component
    return image_size ? image_size : (width * height * 3);
  }(*(int *)&(header[0x22]));

  // Data read from the header of the BMP file
  const unsigned int dataPos = [](int data_pos) {
    return data_pos ? data_pos : 54; // The BMP header is done that way
  }(*(int *)&(header[0x0A]));

  // Create a buffer
  // Actual RGB data
  const auto actual_rgb_data = std::make_unique<char[]>(imageSize);

  // Read the actual data from the file into the buffer
  image_file.read(actual_rgb_data.get(), imageSize);

  // Create one OpenGL texture
  const GLuint textureID = []() {
    GLuint textureID;
    glGenTextures(1, &textureID);
    return textureID;
  }();

  // "Bind" the newly created texture : all future texture functions will modify
  // this texture
  glBindTexture(GL_TEXTURE_2D, textureID);

  // Give the image to OpenGL
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR,
               GL_UNSIGNED_BYTE, actual_rgb_data.get());
  // OpenGL has now copied the data.

  // Poor filtering, or ...
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  // ... nice trilinear filtering ...
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  // ... which requires mipmaps. Generate them automatically.
  glGenerateMipmap(GL_TEXTURE_2D);

  // Return the ID of the texture we just created
  return textureID;
}

GLuint loadDDS(std::string_view imagepath) {

  constexpr unsigned int FOURCC_DXT1 =
      0x31545844; // Equivalent to "DXT1" in ASCII
  constexpr unsigned int FOURCC_DXT3 =
      0x33545844; // Equivalent to "DXT3" in ASCII
  constexpr unsigned int FOURCC_DXT5 =
      0x35545844; // Equivalent to "DXT5" in ASCII

  /* try to open the file */
  std::ifstream image_file(imagepath.data(), std::ios::binary | std::ios::in);
  if (!image_file) {
    std::cerr << imagepath
              << " could not be opened. Are you in the right directory ? Don't "
                 "forget to read the FAQ !\n";
    getchar();
    return 0;
  }

  /* verify the type of file */
  char filecode[4];
  image_file.read(filecode, 4);
  if (strncmp(filecode, "DDS ", 4) != 0) {
    return 0;
  }

  /* get the surface desc */
  char header[124];
  image_file.read(header, 124);

  unsigned int height = *(unsigned int *)&(header[8]);
  unsigned int width = *(unsigned int *)&(header[12]);
  const unsigned int linearSize = *(unsigned int *)&(header[16]);
  const unsigned int mipMapCount = *(unsigned int *)&(header[24]);
  const unsigned int fourCC = *(unsigned int *)&(header[80]);

  /* how big is it going to be including all mipmaps? */
  const unsigned int bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
  const auto buffer = std::make_unique<char[]>(bufsize * sizeof(unsigned char));
  image_file.read(buffer.get(), bufsize);

  // unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
  const unsigned int format = [&fourCC]() {
    switch (fourCC) {
    case FOURCC_DXT1:
      return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
      break;
    case FOURCC_DXT3:
      return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
      break;
    case FOURCC_DXT5:
      return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    }
    return 0;
  }();

  if (!format) {
    return 0;
  }

  // Create one OpenGL texture
  const GLuint textureID = []() {
    GLuint textureID;
    glGenTextures(1, &textureID);
    return textureID;
  }();

  // "Bind" the newly created texture : all future texture functions will
  // modify this texture
  glBindTexture(GL_TEXTURE_2D, textureID);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  const unsigned int blockSize =
      (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
  unsigned int offset = 0;

  /* load the mipmaps */
  for (unsigned int level = 0; level < mipMapCount && (width || height);
       ++level) {
    const unsigned int size =
        ((width + 3) / 4) * ((height + 3) / 4) * blockSize;
    glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size,
                           buffer.get() + offset);

    offset += size;
    width /= 2;
    height /= 2;

    // Deal with Non-Power-Of-Two textures. This code is not included in the
    // webpage to reduce clutter.
    if (width < 1)
      width = 1;
    if (height < 1)
      height = 1;
  }

  return textureID;
}
