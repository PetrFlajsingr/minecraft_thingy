//
// Created by xflajs00 on 24.10.2021.
//

#ifndef MINECRAFT_THINGY_SRC_UTILS_RANDOM_H
#define MINECRAFT_THINGY_SRC_UTILS_RANDOM_H
#include <random>

namespace pf {
inline double generateSeed() {
  std::random_device rd;
  std::mt19937 e{rd()};
  std::uniform_real_distribution<double> dist{0, 10000};
  return dist(e);
}
}// namespace pf

#endif//MINECRAFT_THINGY_SRC_UTILS_RANDOM_H
