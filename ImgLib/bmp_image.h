#pragma once
#include "img_lib.h"

#include <filesystem>

namespace img_lib {
using Path = std::filesystem::path;

bool SaveBMP(const Path& file, const Image& image);
Image LoadBMP(const Path& file);

//File header
static const uint32_t BMP_INDENT = 54;
static const uint32_t BMP_HEAD_PADDING = 0;
//Info header
static const uint32_t BMP_INFO_HEAD_INDENT = 40;
static const uint16_t BMP_PLANE_COUNT = 1;
static const uint16_t BMP_COLOR_DEPTH = 24;
static const uint32_t BMP_COMPRESS_DEPTH = 0;
static const int32_t BMP_H_RES = 11811;
static const int32_t BMP_V_RES = 11811;
static const int32_t BMP_NUM_USING_COLORS = 0; //undefined
static const int32_t BMP_NUM_SIGNIFICANT_COLORS = 0x1000000;

} // namespace img_lib
