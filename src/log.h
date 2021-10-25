//
// Created by xflajs00 on 23.10.2021.
//

#ifndef MINECRAFT_THINGY_SRC_LOG_H
#define MINECRAFT_THINGY_SRC_LOG_H

#include <fmt/format.h>
#include <functional>

namespace details {
inline std::function<void(std::string)> log = [](auto) {};
}

inline void setLogger(std::invocable<std::string> auto &&logger) {
  details::log = logger;
}

template<typename... Args>
void log(std::string_view msg, Args &&...args) {
  details::log(fmt::format(msg, std::forward<Args>(args)...));
}

template<std::ranges::range T>
void logR(std::string msg, T r) {
  std::string str = "[";
  for (auto v : r) {
    str += std::to_string(static_cast<unsigned int>(v)) + ", ";
  }
  str += "]";
  log(msg + ": " + str);
}

#endif//MINECRAFT_THINGY_SRC_LOG_H
