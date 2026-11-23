#pragma once

enum PixelFormat {
    Gray8,
    RGB8,
    RGBA8
};

int bytesPerPixel(PixelFormat);