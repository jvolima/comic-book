#pragma once
#include "images/pixel_format.hpp"
#include <string>

struct ImageInfo {
	int width;
	int height;
	PixelFormat format;
	std::string codecName;
};
