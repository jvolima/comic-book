#include "comic_error.hpp"

class DecodeError : public ComicError {
    DecodeError(const std::string& message);
};