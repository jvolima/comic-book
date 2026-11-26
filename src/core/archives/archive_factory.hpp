#pragma once

#include <memory>
#include <string>
#include <vector>

#include "archives/archive.hpp"
#include "archives/archive_format.hpp"
#include "archives/archive_writer.hpp"

class ArchiveFactory {
private:
	ArchiveFactory() = default;
	std::vector<ArchiveFormat> m_formats;
	const ArchiveFormat* findFormat(const std::string& name) const;
public:
	ArchiveFactory(const ArchiveFactory&) = delete;
	ArchiveFactory& operator=(const ArchiveFactory&) = delete;
	static ArchiveFactory& instance();
	void registerFormat(const ArchiveFormat& format);
	std::unique_ptr<Archive> open(const std::string& path) const;
	std::unique_ptr<ArchiveWriter> createWriter(const std::string& formatName) const;
	ArchiveFormat detect(const std::string& path) const;
	std::vector<std::string> supportedExtensions() const;
};
