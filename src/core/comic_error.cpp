#include "comic_error.hpp"

ComicError::ComicError(const std::string& message) : std::runtime_error(message) {}