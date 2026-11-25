#pragma once

#include <zip.h>
#include <memory>
#include "bytes.hpp"
#include "archives/archive_writer.hpp"

class ZipArchiveWriter : public ArchiveWriter {
private:
	std::unique_ptr<zip_t, decltype(&zip_discard)> m_zip{nullptr, &zip_discard};
public:
	void create(const std::string& path) override;
	void addEntry(const std::string& name, const Bytes& data) override;
	void finish() override;
	void abort() override;
	bool isOpen() const override;
	std::string formatName() const override;
};
