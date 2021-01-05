#pragma once

#include <string>

struct Surface {
  int w;
  int h;
  unsigned char* data;
  int components;

  std::string path;

  Surface(std::string path);
};
