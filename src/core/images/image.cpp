#include "images/image.hpp"

Image::Image(int width, int height, PixelFormat format) : m_width(width), m_height(height), m_format(format), m_stride(m_width * bytesPerPixel(m_format)), m_pixels(m_stride * m_height) {}

int Image::width() {
	return this->m_width;
}

int Image::height() {
	return this->m_height;
}

int Image::stride() {
	return this->m_stride;
}

PixelFormat Image::format() {
	return this->m_format;
}

uint8_t* Image::data() {
	return m_pixels.data();
}

size_t Image::byteSize() {
	return m_pixels.size();
}

bool Image::isNull() {
	return m_pixels.empty();
}
