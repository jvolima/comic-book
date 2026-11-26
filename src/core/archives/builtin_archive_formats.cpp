#include "archives/builtin_archive_formats.hpp"

#include <memory>

#include "archives/archive_factory.hpp"
#include "archives/archive_format.hpp"
#include "archives/zip_archive.hpp"
#include "archives/zip_archive_writer.hpp"

void registerBuiltinArchiveFormats() {
	ArchiveFactory& factory = ArchiveFactory::instance();

	ArchiveFormat zip;
	zip.name = "ZIP";
	zip.extensions = {"cbz", "zip"};
	zip.matches = &ZipArchive::matches;
	zip.createReader = [] { return std::make_unique<ZipArchive>(); };
	zip.createWriter = [] { return std::make_unique<ZipArchiveWriter>(); };
	factory.registerFormat(zip);
}
