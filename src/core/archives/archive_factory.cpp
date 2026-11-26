#include "archives/archive_factory.hpp"

#include <cstddef>
#include <fstream>
#include <ios>

#include "archives/archive_error.hpp"
#include "bytes.hpp"

ArchiveFactory& ArchiveFactory::instance() {
	static ArchiveFactory instance;
	return instance;
}

void ArchiveFactory::registerFormat(const ArchiveFormat& format) {
	if (findFormat(format.name) != nullptr) {
		throw ArchiveError("Format '" + format.name + "' is already registered");
	}
	if (!format.createReader) {
		throw ArchiveError("Format '" + format.name + "' has no reader");
	}
	m_formats.push_back(format);
}

std::unique_ptr<Archive> ArchiveFactory::open(const std::string& path) const {
	std::unique_ptr<Archive> archive = detect(path).createReader();
	archive->open(path);
	return archive;
}

std::unique_ptr<ArchiveWriter> ArchiveFactory::createWriter(const std::string& formatName) const {
	const ArchiveFormat* format = findFormat(formatName);
	if (format == nullptr) {
		throw ArchiveError("Format '" + formatName + "' is not registered");
	}
	if (!format->createWriter) {
		throw ArchiveError("Format '" + formatName + "' cannot be written");
	}
	return format->createWriter();
}

ArchiveFormat ArchiveFactory::detect(const std::string& path) const {
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		throw ArchiveError("Cannot open '" + path + "': file not found or not readable");
	}

	// Enough bytes for every known signature (ZIP: 4, 7z: 6, RAR 5: 8).
	Bytes header(16);
	file.read(reinterpret_cast<char*>(header.data()), static_cast<std::streamsize>(header.size()));
	header.resize(static_cast<std::size_t>(file.gcount()));

	for (const ArchiveFormat& format : m_formats) {
		if (format.matches != nullptr && format.matches(header)) {
			return format;
		}
	}
	throw ArchiveError("Cannot open '" + path + "': not a supported archive format");
}

std::vector<std::string> ArchiveFactory::supportedExtensions() const {
	std::vector<std::string> extensions;
	for (const ArchiveFormat& format : m_formats) {
		extensions.insert(extensions.end(), format.extensions.begin(), format.extensions.end());
	}
	return extensions;
}

const ArchiveFormat* ArchiveFactory::findFormat(const std::string& name) const {
	for (const ArchiveFormat& format : m_formats) {
		if (format.name == name) {
			return &format;
		}
	}
	return nullptr;
}
