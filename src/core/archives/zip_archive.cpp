#include "archives/zip_archive.hpp"

// ZIP magic number: 50 4B 03 04
bool ZipArchive::matches(const Bytes& header) {
	if (header.size() < 4) {
		return false;
	}
	return header[0] == 0x50 && header[1] == 0x4B && header[2] == 0x03 && header[3] == 0x04;
}

bool ZipArchive::open(const std::string& path) {
	int code = 0;
	m_zip.reset(zip_open(path.c_str(), ZIP_RDONLY, &code));
	return m_zip != nullptr;
}

void ZipArchive::close() {
	m_zip.reset();
}

bool ZipArchive::isOpen() const {
	return m_zip != nullptr;
}

std::vector<ArchiveEntry> ZipArchive::listEntries() const {
	std::vector<ArchiveEntry> entries;
	zip_int64_t count = zip_get_num_entries(m_zip.get(), 0);
	if (count <= 0) {
		return entries;
	}
	entries.reserve(count);

	for (zip_int64_t i = 0; i < count; i++) {
		zip_stat_t info;
		zip_stat_init(&info);
		if (zip_stat_index(m_zip.get(), i, 0, &info) != 0) {
			continue;
		}
		ArchiveEntry entry;
		entry.name = info.name;
		entry.size = info.size;
		entry.isDirectory = !entry.name.empty() && entry.name.back() == '/';
		entries.push_back(entry);
	}

	return entries;
}

// TODO: throw ComicError instead of returning empty Bytes on failure
Bytes ZipArchive::readEntry(const std::string& name) const {
	if (!isOpen()) {
		return {};
	}

	zip_stat_t info;
	zip_stat_init(&info);
	if (zip_stat(m_zip.get(), name.c_str(), 0, &info) != 0) {
		return {};
	}

	std::unique_ptr<zip_file_t, decltype(&zip_fclose)> file(zip_fopen(m_zip.get(), name.c_str(), 0), &zip_fclose);
	if (!file) {
		return {};
	}

	Bytes data(info.size);
	zip_int64_t totalRead = zip_fread(file.get(), data.data(), info.size);
	if (totalRead < 0 || static_cast<zip_uint64_t>(totalRead) != info.size) {
		return {};
	}

	// Extra read to reach end of entry: makes libzip check the CRC (-1 = corrupt, > 0 = extra data).
	char extra;
	if (zip_fread(file.get(), &extra, 1) != 0) {
		return {};
	}

	return data;
}

bool ZipArchive::isRandomAccess() const {
	return true;
}

std::string ZipArchive::formatName() const {
	return "ZIP";
}
