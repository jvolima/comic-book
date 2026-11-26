#include "archives/zip_archive.hpp"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <ios>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "archives/archive_entry.hpp"
#include "archives/zip_archive.hpp"
#include "bytes.hpp"
#include "errors/archive_error.hpp"
#include "test_helpers.hpp"

Bytes readHeader(const std::string& path, std::size_t count = 8) {
	std::ifstream file(path, std::ios::binary);
	Bytes header(count);
	file.read(reinterpret_cast<char*>(header.data()), static_cast<std::streamsize>(count));
	header.resize(static_cast<std::size_t>(file.gcount()));
	return header;
}

const ArchiveEntry* findEntry(const std::vector<ArchiveEntry>& entries, const std::string& name) {
	auto it = std::find_if(
	  entries.begin(), entries.end(), [&](const ArchiveEntry& entry) { return entry.name == name; });
	return it == entries.end() ? nullptr : &*it;
}

TEST(ZipArchiveMatches, AcceptsZipSignature) {
	EXPECT_TRUE(ZipArchive::matches({0x50, 0x4B, 0x03, 0x04}));
}

TEST(ZipArchiveMatches, RejectsRarSignature) {
	EXPECT_FALSE(ZipArchive::matches({'R', 'a', 'r', '!', 0x1A, 0x07}));
}

TEST(ZipArchiveMatches, RejectsHeaderShorterThanSignature) {
	EXPECT_FALSE(ZipArchive::matches({0x50, 0x4B}));
	EXPECT_FALSE(ZipArchive::matches({}));
}

TEST(ZipArchiveMatches, DetectsZipByContentNotExtension) {
	EXPECT_TRUE(ZipArchive::matches(readHeader(samplePath("fake_extension.cbr"))));
	EXPECT_FALSE(ZipArchive::matches(readHeader(samplePath("basic.cbr"))));
}

TEST(ZipArchiveOpen, OpensValidZip) {
	ZipArchive archive;

	EXPECT_NO_THROW(archive.open(samplePath("basic.cbz")));
	EXPECT_TRUE(archive.isOpen());
}

TEST(ZipArchiveOpen, FailsOnRarFile) {
	ZipArchive archive;

	EXPECT_THROW(archive.open(samplePath("basic.cbr")), ArchiveError);
	EXPECT_FALSE(archive.isOpen());
}

TEST(ZipArchiveOpen, FailsOnMissingFile) {
	ZipArchive archive;

	EXPECT_THROW(archive.open(samplePath("does_not_exist.cbz")), ArchiveError);
	EXPECT_FALSE(archive.isOpen());
}

TEST(ZipArchiveOpen, CloseIsSafeToCallTwice) {
	ZipArchive archive;

	ASSERT_NO_THROW(archive.open(samplePath("basic.cbz")));

	archive.close();
	archive.close();

	EXPECT_FALSE(archive.isOpen());
}

TEST(ZipArchiveOpen, OpenReplacesPreviouslyOpenedArchive) {
	ZipArchive archive;

	ASSERT_NO_THROW(archive.open(samplePath("basic.cbz")));
	ASSERT_NO_THROW(archive.open(samplePath("different_sizes.cbz")));
	EXPECT_TRUE(archive.isOpen());
	EXPECT_EQ(archive.listEntries().size(), 4u);
}

TEST(ZipArchiveOpen, FailedOpenClosesPreviousArchive) {
	ZipArchive archive;

	ASSERT_NO_THROW(archive.open(samplePath("basic.cbz")));
	EXPECT_THROW(archive.open(samplePath("does_not_exist.cbz")), ArchiveError);
	EXPECT_FALSE(archive.isOpen());
	EXPECT_THROW(archive.listEntries(), ArchiveError);
}

TEST(ZipArchiveOpen, ReopensAfterClose) {
	ZipArchive archive;

	ASSERT_NO_THROW(archive.open(samplePath("basic.cbz")));

	archive.close();

	EXPECT_NO_THROW(archive.open(samplePath("basic.cbz")));
	EXPECT_TRUE(archive.isOpen());
	EXPECT_EQ(archive.listEntries().size(), 10u);
}

TEST(ZipArchiveCorrupt, OpenFailsOnTruncatedArchive) {
	ZipArchive archive;

	EXPECT_TRUE(ZipArchive::matches(readHeader(samplePath("truncated.cbz"))));
	EXPECT_THROW(archive.open(samplePath("truncated.cbz")), ArchiveError);
	EXPECT_FALSE(archive.isOpen());
}

TEST(ZipArchiveCorrupt, ReadEntryThrowsForCorruptEntry) {
	ZipArchive archive;

	ASSERT_NO_THROW(archive.open(samplePath("corrupt_entry.cbz")));
	EXPECT_THROW(archive.readEntry("02.jpg"), ArchiveError);
}

TEST(ZipArchiveCorrupt, CorruptEntryDoesNotAffectOtherEntries) {
	ZipArchive archive;

	ASSERT_NO_THROW(archive.open(samplePath("corrupt_entry.cbz")));
	EXPECT_EQ(archive.listEntries().size(), 2u);
	EXPECT_THROW(archive.readEntry("02.jpg"), ArchiveError);
	EXPECT_EQ(archive.readEntry("01.jpg").size(), 44680u);
}

TEST(ZipArchiveClosed, ListEntriesThrows) {
	ZipArchive archive;

	EXPECT_THROW(archive.listEntries(), ArchiveError);
}

TEST(ZipArchiveClosed, ReadEntryThrows) {
	ZipArchive archive;

	EXPECT_THROW(archive.readEntry("01.jpg"), ArchiveError);
}

TEST(ZipArchiveClosed, ReadAllThrows) {
	ZipArchive archive;

	EXPECT_THROW(archive.readAll([](const ArchiveEntry&, const Bytes&) { return true; }), ArchiveError);
}

class ZipArchiveTest : public ::testing::Test {
protected:
	void SetUp() override {
		ASSERT_NO_THROW(m_archive.open(samplePath("basic.cbz")));
	}
	ZipArchive m_archive;
};

TEST_F(ZipArchiveTest, ListsEveryRawEntry) {
	EXPECT_EQ(m_archive.listEntries().size(), 10u);
}

TEST_F(ZipArchiveTest, ListedEntryHasNameSizeAndFileFlag) {
	std::vector<ArchiveEntry> entries = m_archive.listEntries();
	const ArchiveEntry* entry = findEntry(entries, "01.jpg");

	ASSERT_NE(entry, nullptr);
	EXPECT_EQ(entry->size, 143796u);
	EXPECT_FALSE(entry->isDirectory);
}

TEST_F(ZipArchiveTest, ReadEntryReturnsUncompressedBytes) {
	Bytes data = m_archive.readEntry("01.jpg");

	ASSERT_EQ(data.size(), 143796u);
	EXPECT_EQ(data[0], 0xFF);
	EXPECT_EQ(data[1], 0xD8);
	EXPECT_EQ(data[2], 0xFF);
}

TEST_F(ZipArchiveTest, ReadEntryThrowsForUnknownName) {
	EXPECT_THROW(m_archive.readEntry("missing.jpg"), ArchiveError);
}

TEST_F(ZipArchiveTest, ReadAllVisitsEveryFileInOrder) {
	std::vector<std::string> visited;
	m_archive.readAll([&](const ArchiveEntry& entry, const Bytes& data) {
		EXPECT_EQ(data.size(), entry.size) << entry.name;
		visited.push_back(entry.name);
		return true;
	});

	ASSERT_EQ(visited.size(), 10u);
	EXPECT_EQ(visited.front(), "01.jpg");
}

TEST_F(ZipArchiveTest, ReadAllStopsWhenVisitorReturnsFalse) {
	int calls = 0;
	m_archive.readAll([&](const ArchiveEntry&, const Bytes&) {
		++calls;
		return false;
	});

	EXPECT_EQ(calls, 1);
}

TEST_F(ZipArchiveTest, IsRandomAccess) {
	EXPECT_TRUE(m_archive.isRandomAccess());
}

TEST_F(ZipArchiveTest, FormatNameIsZip) {
	EXPECT_EQ(m_archive.formatName(), "ZIP");
}

TEST(ZipArchiveDirectories, FlagsDirectoryEntries) {
	ZipArchive archive;
	ASSERT_NO_THROW(archive.open(samplePath("mixed.cbz")));
	std::vector<ArchiveEntry> entries = archive.listEntries();
	const ArchiveEntry* folder = findEntry(entries, "subfolder/");

	ASSERT_NE(folder, nullptr);
	EXPECT_TRUE(folder->isDirectory);

	const ArchiveEntry* nested = findEntry(entries, "subfolder/page_05.jpg");

	ASSERT_NE(nested, nullptr);
	EXPECT_FALSE(nested->isDirectory);
}

TEST(ZipArchiveDirectories, ReadAllSkipsDirectories) {
	ZipArchive archive;

	ASSERT_NO_THROW(archive.open(samplePath("mixed.cbz")));

	int calls = 0;
	archive.readAll([&](const ArchiveEntry& entry, const Bytes&) {
		EXPECT_FALSE(entry.isDirectory) << entry.name;
		++calls;
		return true;
	});

	EXPECT_EQ(calls, 8);
}
