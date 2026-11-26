#pragma once

#include <string>

#include "errors/comic_error.hpp"

class ArchiveError : public ComicError {
public:
	ArchiveError(const std::string& message);
};
