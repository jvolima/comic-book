#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "archives/archive.hpp"
#include "archives/archive_writer.hpp"
#include "bytes.hpp"

using SignatureMatcher = bool (*)(const Bytes& header);
using ReaderCreator = std::function<std::unique_ptr<Archive>()>;
using WriterCreator = std::function<std::unique_ptr<ArchiveWriter>()>;

struct ArchiveFormat {
	std::string name;
	std::vector<std::string> extensions;
	SignatureMatcher matches = nullptr;
	ReaderCreator createReader;
	WriterCreator createWriter;
};
