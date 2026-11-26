#pragma once

#include <cstddef>
#include <cstdint>

#include "bytes.hpp"
#include "images/pixel_format.hpp"

class Image {
private:
	int m_width = 0;
	int m_height = 0;
	PixelFormat m_format = PixelFormat::Gray8;
	int m_stride = 0;
	Bytes m_pixels;
public:
	~Image() = default;
	Image() = default;
	Image(int width, int height, PixelFormat format);
	int width();
	int height();
	PixelFormat format();
	int stride();
	uint8_t* data();
	size_t byteSize();
	bool isNull();
};
