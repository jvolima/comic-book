#include "archives/archive_factory.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "archives/archive.hpp"
#include "archives/archive_error.hpp"
#include "archives/archive_format.hpp"
#include "archives/archive_writer.hpp"
#include "archives/builtin_archive_formats.hpp"
#include "archives/zip_archive.hpp"
#include "test_helpers.hpp"

const std::string kReadOnlyFormat = "TEST-READ-ONLY";

ArchiveFactory& factory() {
	static const bool registered = [] {
		registerBuiltinArchiveFormats();
		ArchiveFormat readOnly;
		readOnly.name = kReadOnlyFormat;
		readOnly.extensions = {"tro"};
		readOnly.createReader = [] { return std::make_unique<ZipArchive>(); };
		ArchiveFactory::instance().registerFormat(readOnly);
		return true;
	}();
	(void)registered;
	return ArchiveFactory::instance();
}

bool contains(const std::vector<std::string>& values, const std::string& value) {
	return std::find(values.begin(), values.end(), value) != values.end();
}

TEST(ArchiveFactoryInstance, ReturnsTheSameObject) {
	EXPECT_EQ(&ArchiveFactory::instance(), &ArchiveFactory::instance());
}

TEST(ArchiveFactoryRegister, RejectsDuplicateName) {
	ArchiveFormat duplicate{"ZIP", {"cbz"}, &ZipArchive::matches, [] { return std::make_unique<ZipArchive>(); }, nullptr};
	EXPECT_THROW(factory().registerFormat(duplicate), ArchiveError);
}

TEST(ArchiveFactoryRegister, RejectsFormatWithoutReader) {
	ArchiveFormat noReader{"TEST-NO-READER", {"tnr"}, &ZipArchive::matches, nullptr, nullptr};
	EXPECT_THROW(factory().registerFormat(noReader), ArchiveError);
	EXPECT_FALSE(contains(factory().supportedExtensions(), "tnr"));
}

TEST(ArchiveFactoryRegister, BuiltinFormatsCannotBeRegisteredTwice) {
	factory();
	EXPECT_THROW(registerBuiltinArchiveFormats(), ArchiveError);
}

TEST(ArchiveFactoryDetect, DetectsZip) {
	EXPECT_EQ(factory().detect(samplePath("basic.cbz")).name, "ZIP");
}

TEST(ArchiveFactoryDetect, DetectsByContentNotExtension) {
	EXPECT_EQ(factory().detect(samplePath("fake_extension.cbr")).name, "ZIP");
}

TEST(ArchiveFactoryDetect, ThrowsForUnsupportedFile) {
	EXPECT_THROW(factory().detect(samplePath("README.md")), ArchiveError);
}

TEST(ArchiveFactoryDetect, ThrowsForMissingFile) {
	EXPECT_THROW(factory().detect(samplePath("does_not_exist.cbz")), ArchiveError);
}

TEST(ArchiveFactoryOpen, ReturnsOpenArchive) {
	std::unique_ptr<Archive> archive = factory().open(samplePath("basic.cbz"));
	ASSERT_NE(archive, nullptr);
	EXPECT_TRUE(archive->isOpen());
	EXPECT_EQ(archive->formatName(), "ZIP");
	EXPECT_EQ(archive->listEntries().size(), 10u);
}

TEST(ArchiveFactoryOpen, OpensZipWithWrongExtension) {
	std::unique_ptr<Archive> archive = factory().open(samplePath("fake_extension.cbr"));
	EXPECT_EQ(archive->formatName(), "ZIP");
	EXPECT_EQ(archive->listEntries().size(), 10u);
}

TEST(ArchiveFactoryOpen, ThrowsWhenSignatureMatchesButArchiveIsBroken) {
	EXPECT_THROW(factory().open(samplePath("truncated.cbz")), ArchiveError);
}

TEST(ArchiveFactoryOpen, ThrowsForUnsupportedFile) {
	EXPECT_THROW(factory().open(samplePath("README.md")), ArchiveError);
}

TEST(ArchiveFactoryOpen, ThrowsForMissingFile) {
	EXPECT_THROW(factory().open(samplePath("does_not_exist.cbz")), ArchiveError);
}

TEST(ArchiveFactoryCreateWriter, CreatesZipWriter) {
	std::unique_ptr<ArchiveWriter> writer = factory().createWriter("ZIP");
	ASSERT_NE(writer, nullptr);
	EXPECT_EQ(writer->formatName(), "ZIP");
	EXPECT_FALSE(writer->isOpen());
}

TEST(ArchiveFactoryCreateWriter, ThrowsForUnknownFormat) {
	EXPECT_THROW(factory().createWriter("NOPE"), ArchiveError);
}

TEST(ArchiveFactoryCreateWriter, ThrowsForReadOnlyFormat) {
	EXPECT_THROW(factory().createWriter(kReadOnlyFormat), ArchiveError);
}

TEST(ArchiveFactorySupportedExtensions, ListsEveryRegisteredExtension) {
	std::vector<std::string> extensions = factory().supportedExtensions();
	EXPECT_TRUE(contains(extensions, "cbz"));
	EXPECT_TRUE(contains(extensions, "zip"));
	EXPECT_TRUE(contains(extensions, "tro"));
}
