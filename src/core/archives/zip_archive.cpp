#include "archives/zip_archive.hpp"

#include "errors/archive_error.hpp"

// ZIP magic number: 50 4B 03 04
bool ZipArchive::matches(const Bytes& header) {
	if (header.size() < 4) {
		return false;
	}
	return header[0] == 0x50 && header[1] == 0x4B && header[2] == 0x03 && header[3] == 0x04;
}

void ZipArchive::open(const std::string& path) {
	int code = 0;
	m_zip.reset(zip_open(path.c_str(), ZIP_RDONLY, &code));
	if (!m_zip) {
		zip_error_t error;
		zip_error_init_with_code(&error, code);
		std::string message = "Failed to open ZIP archive '" + path + "': " + zip_error_strerror(&error);
		zip_error_fini(&error);
		throw ArchiveError(message);
	}
}

void ZipArchive::close() {
	m_zip.reset();
}

bool ZipArchive::isOpen() const {
	return m_zip != nullptr;
}

std::vector<ArchiveEntry> ZipArchive::listEntries() const {
	if (!isOpen()) {
		throw ArchiveError("Cannot list entries: archive is not open");
	}

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

Bytes ZipArchive::readEntry(const std::string& name) const {
	if (!isOpen()) {
		throw ArchiveError("Cannot read entry '" + name + "': archive is not open");
	}

	zip_stat_t info;
	zip_stat_init(&info);
	if (zip_stat(m_zip.get(), name.c_str(), 0, &info) != 0) {
		throw ArchiveError("Entry '" + name + "' not found: " + zip_strerror(m_zip.get()));
	}

	std::unique_ptr<zip_file_t, decltype(&zip_fclose)> file(zip_fopen(m_zip.get(), name.c_str(), 0), &zip_fclose);
	if (!file) {
		throw ArchiveError("Failed to open entry '" + name + "': " + zip_strerror(m_zip.get()));
	}

	Bytes data(info.size);
	zip_int64_t totalRead = zip_fread(file.get(), data.data(), info.size);
	if (totalRead < 0) {
		throw ArchiveError("Failed to read entry '" + name + "': " + zip_file_strerror(file.get()));
	}
	if (static_cast<zip_uint64_t>(totalRead) != info.size) {
		throw ArchiveError("Entry '" + name + "' is truncated: read " + std::to_string(totalRead) + " of " + std::to_string(info.size) + " bytes");
	}

	// Extra read to reach end of entry: makes libzip check the CRC (-1 = corrupt, > 0 = extra data).
	char extra;
	zip_int64_t extraRead = zip_fread(file.get(), &extra, 1);
	if (extraRead < 0) {
		throw ArchiveError("Entry '" + name + "' is corrupt: " + zip_file_strerror(file.get()));
	}
	if (extraRead > 0) {
		throw ArchiveError("Entry '" + name + "' has more data than its declared size");
	}

	return data;
}

bool ZipArchive::isRandomAccess() const {
	return true;
}

std::string ZipArchive::formatName() const {
	return "ZIP";
}
