#pragma once

#include <string>
#include <cstdint>

struct ArchiveEntry {
	std::string name;
  std::uint64_t size;
  bool isDirectory;
};
