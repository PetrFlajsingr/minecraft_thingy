//
// Created by xflajs00 on 23.10.2021.
//

#ifndef MINECRAFT_THINGY_SRC_LOG_H
#define MINECRAFT_THINGY_SRC_LOG_H

#include <functional>

namespace details {
inline std::function<void(std::string)> log = [](auto){};
}

inline void setLogger(std::invocable<std::string> auto &&logger) {
  details::log = logger;
}

void log(std::string_view msg, auto ...args) {
  details::log(fmt::format(msg, std::forward<decltype(args)>(args)...));
}

#endif//MINECRAFT_THINGY_SRC_LOG_H
