#include <stdexcept>

class ComicError : public std::runtime_error {
    public:
        ComicError(const std::string& message);
};