#pragma once

#include "archives/archive.hpp"
#include "archives/archive_handle.hpp"
#include "bytes.hpp"

class RarArchive : public Archive {
    private:
        bool m_solid;
        ArchiveHandle m_archive;
    public:
        RarArchive();
        ~RarArchive() = default;
        static bool matches(const Bytes& header);
        void open(const std::string& path) override;
		void close() override;
		bool isOpen() const override;
		std::vector<ArchiveEntry> listEntries() const override;
        Bytes readEntry(const std::string& name) const override;
		bool isRandomAccess() const override;
		std::string formatName() const override;
};
