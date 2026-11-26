#pragma once

#include <string>

#include "errors/comic_error.hpp"

class DecodeError : public ComicError {
public:
	DecodeError(const std::string& message);
};
