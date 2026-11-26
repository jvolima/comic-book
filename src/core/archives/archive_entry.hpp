#pragma once

#include <cstdint>
#include <string>

struct ArchiveEntry {
	std::string name;
	std::uint64_t size = 0;
	bool isDirectory = false;
};
