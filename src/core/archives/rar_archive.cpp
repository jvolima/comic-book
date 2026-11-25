#include "archives/rar_archive.hpp"
#include <archive_entry.h>

RarArchive::RarArchive() : m_archive(nullptr, archive_read_free) {}

bool RarArchive::matches(const Bytes& header) {
    if (header.size() >= 8 &&
        header[0] == 0x52 && header[1] == 0x61 &&
        header[2] == 0x72 && header[3] == 0x21 &&
        header[4] == 0x1A && header[5] == 0x07 &&
        header[6] == 0x01 && header[7] == 0x00) {
        return true;
    }

    if (header.size() >= 7 &&
        header[0] == 0x52 && header[1] == 0x61 &&
        header[2] == 0x72 && header[3] == 0x21 &&
        header[4] == 0x1A && header[5] == 0x07 &&
        header[6] == 0x00) {
        return true;
    }

    return false;
}

bool RarArchive::open(const std::string& path) {
    close();

    this->m_archive.reset(archive_read_new());

    if (!this->m_archive) {
        return false;
    }

    if (archive_read_support_format_rar(this->m_archive.get()) != ARCHIVE_OK || archive_read_support_format_rar5(this->m_archive.get()) != ARCHIVE_OK) {
        close();
        return false;
    }

    if (archive_read_open_filename(this->m_archive.get(), path.c_str(), 10240) != ARCHIVE_OK) {
        close();
        return false;
    }

    return true;
}

void RarArchive::close() {
    if (this->m_archive) {
        archive_read_close(this->m_archive.get());
        this->m_archive.reset();
    }
}

bool RarArchive::isOpen() const {
    return m_archive != nullptr;
}

std::vector<ArchiveEntry> RarArchive::listEntries() const {
    std::vector<ArchiveEntry> entries;

    if (!this->m_archive) {
        return entries;
    }

    struct archive_entry* entry;
    while (archive_read_next_header(this->m_archive.get(), &entry) == ARCHIVE_OK) {
        ArchiveEntry aux;

        std::string path = archive_entry_pathname(entry);
        if (!path.empty() && path.back() == '/') {
            path.pop_back();
        }

        aux.name = path.substr(path.find_last_of('/') + 1);
        aux.size = archive_entry_size(entry);
        aux.isDirectory = (archive_entry_filetype(entry) == AE_IFDIR);

        archive_read_data_skip(this->m_archive.get());

        entries.push_back(aux);
    }
    return entries;
}

bool RarArchive::isRandomAccess() const {
    return false;
}

std::string RarArchive::formatName() const {
    return "RAR";
}

Bytes RarArchive::readEntry(const std::string& name) const {
    // TODO: Implement RAR entry reading.
    return Bytes{};
}