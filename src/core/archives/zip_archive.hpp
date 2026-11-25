#pragma once

#include <zip.h>
#include <memory>
#include "bytes.hpp"
#include "archives/archive.hpp"

class ZipArchive : public Archive {
private:
	std::unique_ptr<zip_t, decltype(&zip_discard)> m_zip{nullptr, &zip_discard};
public:
	static bool matches(const Bytes& header);
	void open(const std::string& path) override;
	void close() override;
	bool isOpen() const override;
	std::vector<ArchiveEntry> listEntries() const override;
	Bytes readEntry(const std::string& name) const override;
	bool isRandomAccess() const override;
	std::string formatName() const override;
};
