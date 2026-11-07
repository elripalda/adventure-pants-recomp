// Adventure Pants Recomp - Xbox 360 STFS package reader
//
// Block layout follows the SDK's StfsContainerDevice (from Xenia): data blocks
// are 4 KB, with a hash table in front of every 170 of them.

#include "stfs.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <system_error>
#include <vector>

namespace adventure_pants {
namespace fs = std::filesystem;

namespace {
constexpr uint64_t kBlockSize = 0x1000;
constexpr uint64_t kPerLevel[3] = {0xAA, 0x70E4, 0x4AF768};

uint32_t ReadBE32(const uint8_t* p) {
  return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3];
}
uint16_t ReadBE16(const uint8_t* p) { return uint16_t((p[0] << 8) | p[1]); }
uint32_t ReadLE24(const uint8_t* p) { return p[0] | (p[1] << 8) | (uint32_t(p[2]) << 16); }
uint32_t ReadBE24(const uint8_t* p) { return (uint32_t(p[0]) << 16) | (p[1] << 8) | p[2]; }

struct Entry {
  std::string name;
  bool dir = false;
  bool contiguous = false;
  uint32_t start = 0;
  int16_t parent = -1;
  uint32_t size = 0;
};

class Package {
 public:
  bool Open(const fs::path& path, std::string* error) {
    file_.open(path, std::ios::binary);
    if (!file_) {
      *error = "Couldn't open the file.";
      return false;
    }
    std::array<uint8_t, 0x971A> hdr{};
    file_.read(reinterpret_cast<char*>(hdr.data()), hdr.size());
    if (file_.gcount() < std::streamsize(hdr.size())) {
      *error = "The file is too small to be an Xbox Live Arcade package.";
      return false;
    }
    if (std::memcmp(hdr.data(), "LIVE", 4) && std::memcmp(hdr.data(), "PIRS", 4) &&
        std::memcmp(hdr.data(), "CON ", 4)) {
      *error = "That isn't an Xbox Live Arcade package.";
      return false;
    }
    header_size_ = ReadBE32(&hdr[0x340]);
    title_id_ = ReadBE32(&hdr[0x360]);
    const uint8_t* vd = &hdr[0x379];
    flags_ = vd[2];
    ft_count_ = uint16_t(vd[3] | (vd[4] << 8));
    ft_block_ = ReadLE24(&vd[5]);
    per_hash_table_ = (flags_ & 1) ? 1 : 2;
    step0_ = kPerLevel[0] + per_hash_table_;
    base_ = (uint64_t(header_size_) + kBlockSize - 1) & ~(kBlockSize - 1);
    for (int i = 0; i < 0x40; ++i) {
      uint16_t c = ReadBE16(&hdr[0x411 + i * 2]);
      if (!c) break;
      name_ += c < 0x80 ? char(c) : '?';
    }
    return true;
  }

  uint32_t title_id() const { return title_id_; }
  const std::string& name() const { return name_; }

  bool ReadEntries(std::vector<Entry>* out, std::string* error) {
    uint32_t blk = ft_block_;
    std::vector<uint8_t> data(kBlockSize);
    for (uint32_t n = 0; n < ft_count_; ++n) {
      if (!ReadBlock(blk, data.data())) {
        *error = "The package's file list is damaged.";
        return false;
      }
      for (size_t i = 0; i < kBlockSize; i += 0x40) {
        const uint8_t* e = &data[i];
        uint8_t fl = e[0x28];
        uint8_t len = fl & 0x3F;
        if (!len) continue;
        Entry ent;
        ent.name.assign(reinterpret_cast<const char*>(e), len);
        ent.dir = fl & 0x80;
        ent.contiguous = fl & 0x40;
        ent.start = ReadLE24(&e[0x2F]);
        ent.parent = int16_t(ReadBE16(&e[0x32]));
        ent.size = ReadBE32(&e[0x34]);
        if (ent.name.find("..") != std::string::npos || ent.name.find('/') != std::string::npos ||
            ent.name.find('\\') != std::string::npos || ent.name.find(':') != std::string::npos) {
          *error = "The package has an unexpected file name.";
          return false;
        }
        out->push_back(std::move(ent));
      }
      blk = NextBlock(blk);
    }
    return true;
  }

  bool ReadBlock(uint32_t idx, uint8_t* out) {
    file_.clear();
    file_.seekg(std::streamoff(BlockOffset(idx)));
    file_.read(reinterpret_cast<char*>(out), kBlockSize);
    return file_.gcount() > 0;
  }

  uint32_t NextBlock(uint32_t idx) {
    uint64_t off = base_ + (HashBlockNumber(idx) << 12) + (idx % kPerLevel[0]) * 0x18 + 0x15;
    uint8_t e[3] = {};
    file_.clear();
    file_.seekg(std::streamoff(off));
    file_.read(reinterpret_cast<char*>(e), 3);
    return ReadBE24(e);
  }

 private:
  uint64_t BlockOffset(uint64_t idx) const {
    uint64_t base = kPerLevel[0];
    uint64_t blk = idx;
    for (int i = 0; i < 3; ++i) {
      blk += ((idx + base) / base) * per_hash_table_;
      if (idx < base) break;
      base *= kPerLevel[0];
    }
    return base_ + (blk << 12);
  }

  uint64_t HashBlockNumber(uint64_t idx) const {
    if (idx < kPerLevel[0]) return 0;
    uint64_t b = (idx / kPerLevel[0]) * step0_;
    b += ((idx / kPerLevel[1]) + 1) * per_hash_table_;
    if (idx < kPerLevel[1]) return b;
    return b + per_hash_table_;
  }

  std::ifstream file_;
  uint32_t header_size_ = 0;
  uint32_t title_id_ = 0;
  uint8_t flags_ = 0;
  uint16_t ft_count_ = 0;
  uint32_t ft_block_ = 0;
  uint64_t per_hash_table_ = 1;
  uint64_t step0_ = 0;
  uint64_t base_ = 0;
  std::string name_;
};
}  // namespace

PackageInfo ReadPackageInfo(const fs::path& package) {
  PackageInfo info;
  Package pkg;
  if (!pkg.Open(package, &info.error)) return info;
  info.ok = true;
  info.title_id = pkg.title_id();
  info.display_name = pkg.name();
  return info;
}

fs::path FindPackage(const fs::path& input, std::string* error) {
  std::error_code ec;
  if (fs::is_regular_file(input, ec)) {
    auto info = ReadPackageInfo(input);
    if (!info.ok) {
      *error = info.error;
      return {};
    }
    if (info.title_id != kTitleId) {
      *error = "That's a different game's package (" + info.display_name + ").";
      return {};
    }
    return input;
  }
  if (!fs::is_directory(input, ec)) {
    *error = "Couldn't find that file or folder.";
    return {};
  }
  fs::recursive_directory_iterator it(input, fs::directory_options::skip_permission_denied, ec), end;
  for (; !ec && it != end; it.increment(ec)) {
    if (it.depth() > 4) {
      it.disable_recursion_pending();
      continue;
    }
    if (!it->is_regular_file(ec)) continue;
    auto size = it->file_size(ec);
    if (ec || size < 0xB000) continue;
    auto info = ReadPackageInfo(it->path());
    if (info.ok && info.title_id == kTitleId) return it->path();
  }
  *error = "No Fancy Pants Adventures package was found in that folder.";
  return {};
}

bool ExtractPackage(const fs::path& package, const fs::path& out_dir, const ProgressFn& progress,
                    std::string* error) {
  Package pkg;
  if (!pkg.Open(package, error)) return false;
  if (pkg.title_id() != kTitleId) {
    *error = "That's a different game's package.";
    return false;
  }
  std::vector<Entry> entries;
  if (!pkg.ReadEntries(&entries, error)) return false;

  auto path_of = [&](size_t i) {
    std::vector<std::string> parts;
    int guard = 0;
    int idx = int(i);
    while (idx >= 0 && idx < int(entries.size()) && guard++ < 64) {
      parts.push_back(entries[idx].name);
      idx = entries[idx].parent;
    }
    fs::path p;
    for (auto r = parts.rbegin(); r != parts.rend(); ++r) p /= *r;
    return p;
  };

  uint64_t total = 0, done = 0;
  for (auto& e : entries)
    if (!e.dir) total += e.size;
  if (!total) total = 1;

  std::error_code ec;
  fs::create_directories(out_dir, ec);
  std::vector<uint8_t> block(kBlockSize);
  for (size_t i = 0; i < entries.size(); ++i) {
    const Entry& e = entries[i];
    fs::path dst = out_dir / path_of(i);
    if (e.dir) {
      fs::create_directories(dst, ec);
      continue;
    }
    fs::create_directories(dst.parent_path(), ec);
    std::ofstream out(dst, std::ios::binary | std::ios::trunc);
    if (!out) {
      *error = "Couldn't write " + dst.filename().string() + ". Is the folder read-only?";
      return false;
    }
    uint64_t remaining = e.size;
    uint32_t blk = e.start;
    uint32_t n = 0;
    uint64_t since_report = 0;
    while (remaining > 0) {
      uint32_t b = e.contiguous ? e.start + n : blk;
      if (!pkg.ReadBlock(b, block.data())) {
        *error = "The package is incomplete or damaged.";
        return false;
      }
      uint64_t chunk = remaining < kBlockSize ? remaining : kBlockSize;
      out.write(reinterpret_cast<const char*>(block.data()), std::streamsize(chunk));
      remaining -= chunk;
      done += chunk;
      since_report += chunk;
      ++n;
      if (!e.contiguous && remaining > 0) blk = pkg.NextBlock(blk);
      if (progress && since_report >= (4u << 20)) {
        since_report = 0;
        progress(done, total, e.name);
      }
    }
    if (!out) {
      *error = "Ran out of space writing " + dst.filename().string() + ".";
      return false;
    }
    if (progress) progress(done, total, e.name);
  }
  if (!fs::exists(out_dir / "default.xex", ec)) {
    *error = "The package didn't contain the game program (default.xex).";
    return false;
  }
  return true;
}

}  // namespace adventure_pants
