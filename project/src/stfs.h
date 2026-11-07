// Adventure Pants Recomp - Xbox 360 STFS package reader (LIVE / PIRS / CON)
#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>

namespace adventure_pants {

constexpr uint32_t kTitleId = 0x58410A9F;

struct PackageInfo {
  bool ok = false;
  uint32_t title_id = 0;
  std::string display_name;
  std::string error;
};

// Reads just the header: is this an STFS package, and for which game?
PackageInfo ReadPackageInfo(const std::filesystem::path& package);

// Finds this game's package: `input` can be the package file itself or any folder
// above it (like the "...(XBLA)" folder from a dump, or its 58410A9F subfolder).
std::filesystem::path FindPackage(const std::filesystem::path& input, std::string* error);

// progress(bytes done, bytes total, current file name)
using ProgressFn = std::function<void(uint64_t, uint64_t, const std::string&)>;

// Unpacks every file in the package into out_dir. Returns false and fills error on failure.
bool ExtractPackage(const std::filesystem::path& package, const std::filesystem::path& out_dir,
                    const ProgressFn& progress, std::string* error);

}  // namespace adventure_pants
