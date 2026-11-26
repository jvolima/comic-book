#pragma once

#include <string>

#include "images/pixel_format.hpp"

struct ImageInfo {
	int width;
	int height;
	PixelFormat format;
	std::string codecName;
};
