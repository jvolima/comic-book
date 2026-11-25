#include <gtest/gtest.h>

#include <fstream>
#include <algorithm>
#include <vector>

#include "archives/rar_archive.hpp"

namespace {
	std::string samplePath(const std::string& fileName) {
		return std::string(SAMPLES_DIR) + "/" + fileName;
	}

	Bytes readHeader(const std::string& path, std::size_t count = 8) {
		std::ifstream file(path, std::ios::binary);
		Bytes header(count);
		file.read(reinterpret_cast<char*>(header.data()), static_cast<std::streamsize>(count));
		header.resize(static_cast<std::size_t>(file.gcount()));
		return header;
	}

	const ArchiveEntry* findEntry(const std::vector<ArchiveEntry>& entries, const std::string& name) {
		auto it = std::find_if(
			entries.begin(), entries.end(), [&](const ArchiveEntry& entry) 
			{ return entry.name == name; }
		);
		return it == entries.end() ? nullptr : &*it;
	}
}

TEST(RarArchiveMatches, AcceptsRar4Signature) {
    EXPECT_TRUE(RarArchive::matches({
        0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00
    }));
}

TEST(RarArchiveMatches, AcceptsRar5Signature) {
    EXPECT_TRUE(RarArchive::matches({
        0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x01, 0x00
    }));
}

TEST(RarArchiveMatches, AcceptsRar4SignatureWithAdditionalBytes) {
    EXPECT_TRUE(RarArchive::matches({
        0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00, 0x00
    }));
}

TEST(RarArchiveMatches, RejectsZipSignature) {
    EXPECT_FALSE(RarArchive::matches({
        0x50, 0x4B, 0x03, 0x04
    }));
}

TEST(RarArchiveMatches, RejectsHeaderShorterThanSignature) {
    EXPECT_FALSE(RarArchive::matches({0x52, 0x61, 0x72}));
    EXPECT_FALSE(RarArchive::matches({}));
}

TEST(RarArchiveMatches, DetectsRarByContentNotExtension) {
    EXPECT_TRUE(RarArchive::matches(readHeader(samplePath("basic.cbr"))));
    EXPECT_FALSE(RarArchive::matches(readHeader(samplePath("fake_extension.cbr"))));
}

TEST(RarArchiveOpen, OpensValidRar) {
    RarArchive archive;
    EXPECT_TRUE(archive.open(samplePath("basic.cbr")));
    EXPECT_TRUE(archive.isOpen());
}

TEST(RarArchiveOpen, FailsOnZipFile) {
    RarArchive archive;
    EXPECT_FALSE(archive.open(samplePath("basic.cbz")));
    EXPECT_FALSE(archive.isOpen());
}

TEST(RarArchiveOpen, FailsOnMissingFile) {
    RarArchive archive;
    EXPECT_FALSE(archive.open(samplePath("does_not_exist.cbr")));
    EXPECT_FALSE(archive.isOpen());
}

TEST(RarArchiveOpen, CloseIsSafeToCallTwice) {
    RarArchive archive;
    ASSERT_TRUE(archive.open(samplePath("basic.cbr")));

    archive.close();
    archive.close();

    EXPECT_FALSE(archive.isOpen());
}

TEST(RarArchiveOpen, OpenReplacesPreviouslyOpenedArchive) {
    RarArchive archive;
    ASSERT_TRUE(archive.open(samplePath("basic.cbr")));

    ASSERT_TRUE(archive.open(samplePath("basic.cbr")));

    EXPECT_TRUE(archive.isOpen());
}

TEST(RarArchiveOpen, FailedOpenClosesPreviousArchive) {
    RarArchive archive;
    ASSERT_TRUE(archive.open(samplePath("basic.cbr")));

    EXPECT_FALSE(archive.open(samplePath("does_not_exist.cbr")));
    EXPECT_FALSE(archive.isOpen());
    EXPECT_TRUE(archive.listEntries().empty());
}

TEST(RarArchiveOpen, ReopensAfterClose) {
    RarArchive archive;
    ASSERT_TRUE(archive.open(samplePath("basic.cbr")));

    archive.close();

    EXPECT_TRUE(archive.open(samplePath("basic.cbr")));
    EXPECT_TRUE(archive.isOpen());
}

class RarArchiveTest : public ::testing::Test {
    protected:
        void SetUp() override {
            ASSERT_TRUE(m_archive.open(samplePath("basic.cbr")));
        }

        RarArchive m_archive;
};

TEST_F(RarArchiveTest, ListsEveryRawEntry) {
    EXPECT_EQ(m_archive.listEntries().size(), 5u);
}

TEST_F(RarArchiveTest, ListedEntryHasNameSizeAndFileFlag) {
    std::vector<ArchiveEntry> entries = m_archive.listEntries();

    const ArchiveEntry* entry = findEntry(entries, "01.jpg");

    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry->size, 143796u);
    EXPECT_FALSE(entry->isDirectory);
}

TEST_F(RarArchiveTest, ListsAllExpectedNames) {
    std::vector<ArchiveEntry> entries = m_archive.listEntries();

    std::vector<std::string> expectedNames = {
        "01.jpg",
        "02.jpg",
        "03.jpg",
        "04.jpg",
        "05.jpg"
    };

    ASSERT_EQ(entries.size(), expectedNames.size());

    for (std::size_t i = 0; i < expectedNames.size(); ++i) {
        EXPECT_EQ(entries[i].name, expectedNames[i]);
    }
}

TEST_F(RarArchiveTest, IsNotRandomAccess) {
    EXPECT_FALSE(m_archive.isRandomAccess());
}

TEST_F(RarArchiveTest, FormatNameIsRar) {
    EXPECT_EQ(m_archive.formatName(), "RAR");
}

TEST(RarArchiveClosed, ListEntriesReturnsEmpty) {
    RarArchive archive;
    EXPECT_TRUE(archive.listEntries().empty());
}