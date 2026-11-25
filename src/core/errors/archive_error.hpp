#include "errors/comic_error.hpp"

class ArchiveError : public ComicError {
    ArchiveError(const std::string& message);
};