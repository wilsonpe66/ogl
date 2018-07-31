#pragma once

#include <string_view>

void initText2D(std::string_view texturePath);
void printText2D(std::string_view text, int x, int y, int size);
void cleanupText2D();
