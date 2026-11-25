#pragma once

#include <string>
#include "bytes.hpp"

class ArchiveWriter {
public:
	ArchiveWriter() = default;
	ArchiveWriter(const ArchiveWriter&) = delete;
	ArchiveWriter& operator=(const ArchiveWriter&) = delete;
	virtual ~ArchiveWriter() = default;
	virtual void create(const std::string& path) = 0;
	virtual void addEntry(const std::string& name, const Bytes& data) = 0;
	virtual void finish() = 0;
	virtual void abort() = 0;
	virtual bool isOpen() const = 0;
	virtual std::string formatName() const = 0;
};
