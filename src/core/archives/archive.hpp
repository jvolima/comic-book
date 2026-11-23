#pragma once

#include <functional>
#include <string>
#include <vector>
#include "bytes.hpp"
#include "archives/archive_entry.hpp"

using EntryVisitor = std::function<bool(const ArchiveEntry&, const Bytes&)>;

class Archive {
	public:
		Archive() = default;
		Archive(const Archive&) = delete;
		Archive& operator=(const Archive&) = delete;
		virtual ~Archive() = default;
		virtual bool open(const std::string& path) = 0;
		virtual void close() = 0;
		virtual bool isOpen() const = 0;
		virtual std::vector<ArchiveEntry> listEntries() const = 0;
		virtual Bytes readEntry(const std::string& name) const = 0;
		virtual void readAll(const EntryVisitor& visitor) const {
			for (const ArchiveEntry& entry : listEntries()) {
				if (entry.isDirectory) {
					continue;
				}
				if (!visitor(entry, readEntry(entry.name))) {
					break;
				}
			}
		}
		virtual bool isRandomAccess() const = 0;
		virtual std::string formatName() const = 0;
};
