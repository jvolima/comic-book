#include <cstdlib>
#include <cstring>
#include <filesystem>
#include "archives/zip_archive_writer.hpp"
#include "archives/archive_error.hpp"

void ZipArchiveWriter::create(const std::string& path) {
	// zip_open() does not touch the disk, so a missing directory would only
	// be detected in finish(): check it now.
	std::filesystem::path parent = std::filesystem::path(path).parent_path();
	std::error_code ignored;
	if (!parent.empty() && !std::filesystem::is_directory(parent, ignored)) {
		m_zip.reset();
		throw ArchiveError("Failed to create ZIP archive '" + path + "': directory '" + parent.string() + "' does not exist");
	}

	int code = 0;
	m_zip.reset(zip_open(path.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &code));
	if (!m_zip) {
		zip_error_t error;
		zip_error_init_with_code(&error, code);
		std::string message = "Failed to create ZIP archive '" + path + "': " + zip_error_strerror(&error);
		zip_error_fini(&error);
		throw ArchiveError(message);
	}
}

void ZipArchiveWriter::addEntry(const std::string& name, const Bytes& data) {
	if (!isOpen()) {
		throw ArchiveError("Cannot add entry '" + name + "': archive is not open");
	}

	void* copy = nullptr;
	if (!data.empty()) {
		copy = std::malloc(data.size());
		if (!copy) {
			throw ArchiveError("Cannot add entry '" + name + "': out of memory");
		}
		std::memcpy(copy, data.data(), data.size());
	}

	zip_source_t* source = zip_source_buffer(m_zip.get(), copy, data.size(), 1);
	if (!source) {
		std::free(copy);
		throw ArchiveError("Failed to add entry '" + name + "': " + zip_strerror(m_zip.get()));
	}

	if (zip_file_add(m_zip.get(), name.c_str(), source, ZIP_FL_ENC_UTF_8) < 0) {
		zip_source_free(source);
		throw ArchiveError("Failed to add entry '" + name + "': " + zip_strerror(m_zip.get()));
	}
}

void ZipArchiveWriter::finish() {
	if (!isOpen()) {
		throw ArchiveError("Cannot finish: archive is not open");
	}

	// With no entries, zip_close() writes nothing and deletes any existing file.
	if (zip_get_num_entries(m_zip.get(), 0) == 0) {
		m_zip.reset();
		throw ArchiveError("Cannot finish: archive has no entries");
	}

	if (zip_close(m_zip.get()) < 0) {
		std::string message = std::string("Failed to write ZIP archive: ") + zip_strerror(m_zip.get());
		m_zip.reset();
		throw ArchiveError(message);
	}
	m_zip.release();
}

void ZipArchiveWriter::abort() {
	m_zip.reset();
}

bool ZipArchiveWriter::isOpen() const {
	return m_zip != nullptr;
}

std::string ZipArchiveWriter::formatName() const {
	return "ZIP";
}
