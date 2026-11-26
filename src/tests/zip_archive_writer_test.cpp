#include "archives/zip_archive_writer.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

#include "archives/archive_entry.hpp"
#include "archives/archive_error.hpp"
#include "archives/zip_archive.hpp"
#include "bytes.hpp"
#include "test_helpers.hpp"

namespace fs = std::filesystem;

Bytes makeBytes(std::size_t size, std::uint8_t seed) {
	Bytes bytes(size);
	for (std::size_t i = 0; i < size; i++) {
		bytes[i] = static_cast<std::uint8_t>(seed + i * 31);
	}
	return bytes;
}

std::vector<std::string> entryNames(const ZipArchive& archive) {
	std::vector<std::string> names;
	for (const ArchiveEntry& entry : archive.listEntries()) {
		names.push_back(entry.name);
	}
	return names;
}

class ZipArchiveWriterTest : public ::testing::Test {
protected:
	void SetUp() override {
		const ::testing::TestInfo* info = ::testing::UnitTest::GetInstance()->current_test_info();
		m_dir = fs::temp_directory_path() / (std::string("comic_zip_writer_") + info->test_suite_name() + "_" + info->name());
		fs::remove_all(m_dir);
		fs::create_directories(m_dir);
	}

	void TearDown() override {
		std::error_code ignored;
		fs::remove_all(m_dir, ignored);
	}

	std::string pathFor(const std::string& fileName) const {
		return (m_dir / fileName).string();
	}

	fs::path m_dir;
	ZipArchiveWriter m_writer;
};

TEST_F(ZipArchiveWriterTest, FormatNameIsZip) {
	EXPECT_EQ(m_writer.formatName(), "ZIP");
}

TEST_F(ZipArchiveWriterTest, IsNotOpenBeforeCreate) {
	EXPECT_FALSE(m_writer.isOpen());
}

TEST_F(ZipArchiveWriterTest, IsOpenAfterCreate) {
	ASSERT_NO_THROW(m_writer.create(pathFor("book.cbz")));
	EXPECT_TRUE(m_writer.isOpen());
}

TEST_F(ZipArchiveWriterTest, WrittenArchiveIsReadableByZipArchive) {
	std::string path = pathFor("book.cbz");
	Bytes page1 = makeBytes(1000, 1);
	Bytes page2 = makeBytes(5000, 2);

	m_writer.create(path);
	m_writer.addEntry("001.jpg", page1);
	m_writer.addEntry("002.jpg", page2);
	ASSERT_NO_THROW(m_writer.finish());
	EXPECT_FALSE(m_writer.isOpen());

	ZipArchive archive;
	ASSERT_NO_THROW(archive.open(path));
	EXPECT_EQ(archive.readEntry("001.jpg"), page1);
	EXPECT_EQ(archive.readEntry("002.jpg"), page2);
}

TEST_F(ZipArchiveWriterTest, WrittenArchiveHasZipSignature) {
	std::string path = pathFor("book.cbz");
	m_writer.create(path);
	m_writer.addEntry("001.jpg", makeBytes(10, 0));
	m_writer.finish();

	ZipArchive archive;
	ASSERT_NO_THROW(archive.open(path));
	EXPECT_EQ(archive.formatName(), m_writer.formatName());
}

TEST_F(ZipArchiveWriterTest, KeepsEntriesInTheOrderTheyWereAdded) {
	std::string path = pathFor("book.cbz");
	m_writer.create(path);
	m_writer.addEntry("002.jpg", makeBytes(10, 2));
	m_writer.addEntry("001.jpg", makeBytes(10, 1));
	m_writer.addEntry("003.jpg", makeBytes(10, 3));
	m_writer.finish();

	ZipArchive archive;
	ASSERT_NO_THROW(archive.open(path));
	EXPECT_EQ(entryNames(archive), (std::vector<std::string>{"002.jpg", "001.jpg", "003.jpg"}));
}

TEST_F(ZipArchiveWriterTest, CopiesDataSoCallerBufferCanBeDestroyedBeforeFinish) {
	std::string path = pathFor("book.cbz");
	Bytes expected = makeBytes(100000, 7);

	m_writer.create(path);
	{
		Bytes temporary = expected;
		m_writer.addEntry("001.jpg", temporary);
	}
	Bytes reusesFreedMemory(100000, 0xAA);
	m_writer.addEntry("002.jpg", reusesFreedMemory);
	m_writer.finish();

	ZipArchive archive;
	ASSERT_NO_THROW(archive.open(path));
	EXPECT_EQ(archive.readEntry("001.jpg"), expected);
}

TEST_F(ZipArchiveWriterTest, WritesEmptyEntry) {
	std::string path = pathFor("book.cbz");
	m_writer.create(path);
	m_writer.addEntry("empty.txt", Bytes{});
	m_writer.addEntry("001.jpg", makeBytes(10, 1));
	m_writer.finish();

	ZipArchive archive;
	ASSERT_NO_THROW(archive.open(path));
	EXPECT_TRUE(archive.readEntry("empty.txt").empty());
}

TEST_F(ZipArchiveWriterTest, WritesEntriesInsideFolders) {
	std::string path = pathFor("book.cbz");
	Bytes page = makeBytes(10, 5);
	m_writer.create(path);
	m_writer.addEntry("chapter1/001.jpg", page);
	m_writer.finish();

	ZipArchive archive;
	ASSERT_NO_THROW(archive.open(path));
	EXPECT_EQ(archive.readEntry("chapter1/001.jpg"), page);
}

TEST_F(ZipArchiveWriterTest, KeepsUtf8EntryNames) {
	std::string path = pathFor("book.cbz");
	m_writer.create(path);
	m_writer.addEntry("página_01.jpg", makeBytes(10, 1));
	m_writer.finish();

	ZipArchive archive;
	ASSERT_NO_THROW(archive.open(path));
	EXPECT_EQ(entryNames(archive), (std::vector<std::string>{"página_01.jpg"}));
}

TEST_F(ZipArchiveWriterTest, DuplicateEntryNameThrows) {
	std::string path = pathFor("book.cbz");
	Bytes first = makeBytes(10, 1);
	m_writer.create(path);
	m_writer.addEntry("001.jpg", first);
	EXPECT_THROW(m_writer.addEntry("001.jpg", makeBytes(20, 2)), ArchiveError);

	EXPECT_TRUE(m_writer.isOpen());
	ASSERT_NO_THROW(m_writer.finish());
	ZipArchive archive;
	ASSERT_NO_THROW(archive.open(path));
	EXPECT_EQ(archive.listEntries().size(), 1u);
	EXPECT_EQ(archive.readEntry("001.jpg"), first);
}

TEST_F(ZipArchiveWriterTest, CreateOverwritesExistingArchive) {
	std::string path = pathFor("book.cbz");
	m_writer.create(path);
	m_writer.addEntry("old.jpg", makeBytes(10, 1));
	m_writer.finish();

	m_writer.create(path);
	m_writer.addEntry("new.jpg", makeBytes(10, 2));
	m_writer.finish();

	ZipArchive archive;
	ASSERT_NO_THROW(archive.open(path));
	EXPECT_EQ(entryNames(archive), (std::vector<std::string>{"new.jpg"}));
}

TEST_F(ZipArchiveWriterTest, CreateOverwritesFileThatIsNotZip) {
	std::string path = pathFor("book.cbz");
	fs::copy_file(samplePath("basic.cbr"), path);

	ASSERT_NO_THROW(m_writer.create(path));
	m_writer.addEntry("001.jpg", makeBytes(10, 1));
	ASSERT_NO_THROW(m_writer.finish());

	ZipArchive archive;
	ASSERT_NO_THROW(archive.open(path));
	EXPECT_EQ(entryNames(archive), (std::vector<std::string>{"001.jpg"}));
}

TEST_F(ZipArchiveWriterTest, CreateFailsInMissingDirectory) {
	EXPECT_THROW(m_writer.create(pathFor("missing_dir/book.cbz")), ArchiveError);
	EXPECT_FALSE(m_writer.isOpen());
}

TEST_F(ZipArchiveWriterTest, CreateFailsWhenParentIsAFile) {
	std::string file = pathFor("not_a_dir");
	fs::copy_file(samplePath("basic.cbz"), file);
	EXPECT_THROW(m_writer.create(file + "/book.cbz"), ArchiveError);
	EXPECT_FALSE(m_writer.isOpen());
}

TEST_F(ZipArchiveWriterTest, FailedCreateDiscardsUnfinishedArchive) {
	std::string first = pathFor("first.cbz");
	m_writer.create(first);
	m_writer.addEntry("001.jpg", makeBytes(10, 1));

	EXPECT_THROW(m_writer.create(pathFor("missing_dir/book.cbz")), ArchiveError);
	EXPECT_FALSE(m_writer.isOpen());
	EXPECT_FALSE(fs::exists(first));
}

TEST_F(ZipArchiveWriterTest, CreateAcceptsPathWithoutDirectory) {
	fs::path previous = fs::current_path();
	fs::current_path(m_dir);
	EXPECT_NO_THROW(m_writer.create("book.cbz"));
	m_writer.addEntry("001.jpg", makeBytes(10, 1));
	EXPECT_NO_THROW(m_writer.finish());
	fs::current_path(previous);
	EXPECT_TRUE(fs::exists(m_dir / "book.cbz"));
}

TEST_F(ZipArchiveWriterTest, FinishWithoutEntriesThrowsAndClosesWriter) {
	std::string path = pathFor("book.cbz");
	m_writer.create(path);
	EXPECT_THROW(m_writer.finish(), ArchiveError);
	EXPECT_FALSE(m_writer.isOpen());
	EXPECT_FALSE(fs::exists(path));
}

TEST_F(ZipArchiveWriterTest, FinishWithoutEntriesKeepsExistingArchive) {
	std::string path = pathFor("book.cbz");
	m_writer.create(path);
	m_writer.addEntry("old.jpg", makeBytes(10, 1));
	m_writer.finish();

	m_writer.create(path);
	EXPECT_THROW(m_writer.finish(), ArchiveError);

	ZipArchive archive;
	ASSERT_NO_THROW(archive.open(path));
	EXPECT_EQ(entryNames(archive), (std::vector<std::string>{"old.jpg"}));
}

TEST_F(ZipArchiveWriterTest, FailedAddDoesNotCountAsEntry) {
	m_writer.create(pathFor("book.cbz"));
	m_writer.addEntry("001.jpg", makeBytes(10, 1));
	EXPECT_THROW(m_writer.addEntry("001.jpg", makeBytes(10, 2)), ArchiveError);
	EXPECT_NO_THROW(m_writer.finish());
}

TEST_F(ZipArchiveWriterTest, CreateDiscardsUnfinishedArchive) {
	std::string first = pathFor("first.cbz");
	std::string second = pathFor("second.cbz");
	m_writer.create(first);
	m_writer.addEntry("001.jpg", makeBytes(10, 1));

	m_writer.create(second);
	m_writer.addEntry("001.jpg", makeBytes(10, 2));
	m_writer.finish();

	EXPECT_FALSE(fs::exists(first));
	EXPECT_TRUE(fs::exists(second));
}

TEST_F(ZipArchiveWriterTest, CanBeReusedAfterFinish) {
	std::string first = pathFor("first.cbz");
	std::string second = pathFor("second.cbz");
	m_writer.create(first);
	m_writer.addEntry("001.jpg", makeBytes(10, 1));
	m_writer.finish();

	ASSERT_NO_THROW(m_writer.create(second));
	m_writer.addEntry("001.jpg", makeBytes(10, 2));
	ASSERT_NO_THROW(m_writer.finish());

	EXPECT_TRUE(fs::exists(first));
	EXPECT_TRUE(fs::exists(second));
}

TEST_F(ZipArchiveWriterTest, AbortLeavesNoFile) {
	std::string path = pathFor("book.cbz");
	m_writer.create(path);
	m_writer.addEntry("001.jpg", makeBytes(10, 1));
	m_writer.abort();

	EXPECT_FALSE(m_writer.isOpen());
	EXPECT_FALSE(fs::exists(path));
}

TEST_F(ZipArchiveWriterTest, AbortKeepsExistingArchiveUntouched) {
	std::string path = pathFor("book.cbz");
	m_writer.create(path);
	m_writer.addEntry("old.jpg", makeBytes(10, 1));
	m_writer.finish();

	m_writer.create(path);
	m_writer.addEntry("new.jpg", makeBytes(10, 2));
	m_writer.abort();

	ZipArchive archive;
	ASSERT_NO_THROW(archive.open(path));
	EXPECT_EQ(entryNames(archive), (std::vector<std::string>{"old.jpg"}));
}

TEST_F(ZipArchiveWriterTest, AbortIsSafeWhenNotOpen) {
	EXPECT_NO_THROW(m_writer.abort());
	EXPECT_NO_THROW(m_writer.abort());
	EXPECT_FALSE(m_writer.isOpen());
}

TEST_F(ZipArchiveWriterTest, DestroyingUnfinishedWriterLeavesNoFile) {
	std::string path = pathFor("book.cbz");
	{
		ZipArchiveWriter writer;
		writer.create(path);
		writer.addEntry("001.jpg", makeBytes(10, 1));
	}
	EXPECT_FALSE(fs::exists(path));
}

TEST_F(ZipArchiveWriterTest, FinishFailureThrowsAndClosesWriter) {
	fs::path subdir = m_dir / "removed";
	fs::create_directories(subdir);
	m_writer.create((subdir / "book.cbz").string());
	m_writer.addEntry("001.jpg", makeBytes(10, 1));
	fs::remove_all(subdir);

	EXPECT_THROW(m_writer.finish(), ArchiveError);
	EXPECT_FALSE(m_writer.isOpen());
}

TEST_F(ZipArchiveWriterTest, RewritesSampleBookWithIdenticalPages) {
	ZipArchive source;
	ASSERT_NO_THROW(source.open(samplePath("basic.cbz")));

	std::string path = pathFor("copy.cbz");
	m_writer.create(path);
	source.readAll([&](const ArchiveEntry& entry, const Bytes& data) {
		m_writer.addEntry(entry.name, data);
		return true;
	});
	ASSERT_NO_THROW(m_writer.finish());

	ZipArchive copy;
	ASSERT_NO_THROW(copy.open(path));
	EXPECT_EQ(entryNames(copy), entryNames(source));
	for (const std::string& name : entryNames(source)) {
		EXPECT_EQ(copy.readEntry(name), source.readEntry(name)) << name;
	}
}

TEST_F(ZipArchiveWriterTest, AddEntryAfterFinishThrows) {
	m_writer.create(pathFor("book.cbz"));
	m_writer.addEntry("001.jpg", makeBytes(10, 1));
	m_writer.finish();
	EXPECT_THROW(m_writer.addEntry("002.jpg", makeBytes(10, 2)), ArchiveError);
}

TEST_F(ZipArchiveWriterTest, FinishTwiceThrows) {
	m_writer.create(pathFor("book.cbz"));
	m_writer.addEntry("001.jpg", makeBytes(10, 1));
	m_writer.finish();
	EXPECT_THROW(m_writer.finish(), ArchiveError);
}

TEST(ZipArchiveWriterClosed, AddEntryThrows) {
	ZipArchiveWriter writer;
	EXPECT_THROW(writer.addEntry("001.jpg", Bytes{1, 2, 3}), ArchiveError);
}

TEST(ZipArchiveWriterClosed, FinishThrows) {
	ZipArchiveWriter writer;
	EXPECT_THROW(writer.finish(), ArchiveError);
}
