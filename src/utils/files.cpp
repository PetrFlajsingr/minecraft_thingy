//
// Created by xflajs00 on 22.10.2021.
//

#include "files.h"
#include <fstream>

std::filesystem::path pf::getExeFolder() {
  const auto currentPath = std::filesystem::current_path();
  return absolute(currentPath);
}
std::optional<std::string> pf::readFile(const std::filesystem::path &path) {
  std::ifstream ifstream{path};
  if (!ifstream.is_open()) {
    return std::nullopt;
  }
  return std::string{std::istreambuf_iterator<char>{ifstream}, std::istreambuf_iterator<char>{}};
}
std::optional<std::vector<std::byte>> pf::readBinFile(const std::filesystem::path &path) {
  std::ifstream ifstream{path, std::ios::binary | std::ios::ate};
  if (!ifstream.is_open()) {
    return std::nullopt;
  }
  std::ifstream::pos_type pos = ifstream.tellg();

  std::vector<char>  charData(pos);

  ifstream.seekg(0, std::ios::beg);
  ifstream.read(&charData[0], pos);

  std::vector<std::byte> result;
  result.reserve(charData.size());
  for (auto ch : charData) {
    result.emplace_back(*reinterpret_cast<const std::byte*>(&ch));
  }
  return result;
}
