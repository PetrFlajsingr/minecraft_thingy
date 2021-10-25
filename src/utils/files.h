/**
* @file files.h
* @brief File loading/saving utils.
* @author Petr Flajšingr
* @date 25.10.2021
*/
#ifndef OPENGL_TEMPLATE_SRC_UTILS_FILES_H
#define OPENGL_TEMPLATE_SRC_UTILS_FILES_H

#include <filesystem>
#include <fstream>
#include <string>

namespace pf {

/**
 * @return absolute path to folder of current executable
 */
[[nodiscard]] std::filesystem::path getExeFolder();

[[nodiscard]] std::optional<std::string> readFile(const std::filesystem::path &path);

[[nodiscard]] std::optional<std::vector<std::byte>> readBinFile(const std::filesystem::path &path);

inline void createFile(const std::filesystem::path &path, std::string data) {
  std::ofstream ofstream{path};
  ofstream << data;
}

template <typename T>
void createFile(const std::filesystem::path& path, const std::vector<T> &data) {
  std::ofstream ofstream{path};
  ofstream.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(T));
}

}// namespace pf

#endif//OPENGL_TEMPLATE_SRC_UTILS_FILES_H
