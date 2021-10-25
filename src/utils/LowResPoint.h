//
// Created by xflajs00 on 25.10.2021.
//

#ifndef MINECRAFT_THINGY_SRC_UTILS_LOWRESPOINT_H
#define MINECRAFT_THINGY_SRC_UTILS_LOWRESPOINT_H

#include <functional>
#include <glm/vec3.hpp>

namespace pf::mc {

struct LowResPoint {
  inline LowResPoint() {}
  template<typename T> // this should be std::integral, but MSVC internal error...
  LowResPoint(T x, T y, T z) : x(static_cast<std::uint8_t>(x)),
                               y(static_cast<std::uint8_t>(y)),
                               z(static_cast<std::uint8_t>(z)) {}
  std::uint8_t x{};
  std::uint8_t y{};
  std::uint8_t z{};

  inline operator glm::ivec3() const {
    return {x, y, z};
  }

  inline bool operator==(const LowResPoint &rhs) const {
    return x == rhs.x && y == rhs.y && z == rhs.z;
  }
  inline bool operator!=(const LowResPoint &rhs) const {
    return !(rhs == *this);
  }
};
}// namespace pf::mc

namespace std {
template<>
struct hash<pf::mc::LowResPoint> {
  std::size_t operator()(const pf::mc::LowResPoint &k) const {
    return k.x + k.y * 255 + k.z * 255 * 255;
  }
};

}// namespace std

#endif//MINECRAFT_THINGY_SRC_UTILS_LOWRESPOINT_H
