#ifndef PNG_LOADER_H
#define PNG_LOADER_H
#include <png.h>
#include <string>
#include <vector>

bool LoadImagePNG(const std::string& name, std::vector<unsigned int>& buffer, unsigned int& width, unsigned int& height);

#endif
