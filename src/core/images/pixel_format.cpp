#include "images/pixel_format.hpp"

int bytesPerPixel(PixelFormat format) {
	switch (format) {
		case PixelFormat::Gray8:
			return 1;
		case PixelFormat::RGB8:
			return 3;
		case PixelFormat::RGBA8:
			return 4;
	}
	return 0;
}
