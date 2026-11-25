#include <archive.h>
#include <memory>

using ArchiveHandle = std::unique_ptr<archive, decltype(&archive_read_free)>;