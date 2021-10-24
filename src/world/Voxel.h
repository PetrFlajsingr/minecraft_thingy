//
// Created by xflajs00 on 23.10.2021.
//

#ifndef OPENGL_TEMPLATE_SRC_WORLD_VOXEL_H
#define OPENGL_TEMPLATE_SRC_WORLD_VOXEL_H

#include <chrono>
#include <glm/vec4.hpp>

namespace pf::mc {

struct alignas(1) Voxel {
  enum class Type : std::uint8_t {
    Empty = 0, Grass = 1, Gravel = 2, Dirt = 3, Ice = 4
  };
  Type type;

  [[nodiscard]] inline std::chrono::milliseconds getDestructionTime() const {
    using namespace std::chrono_literals;
    switch (type) {
      case Type::Grass: return 700ms;
      case Type::Gravel: return 1500ms;
      case Type::Dirt: return 400ms;
      case Type::Ice: return 500ms;
    }
    assert(false && "If this gets called on Empty something is very wrong");
  }

  using Vertex = glm::tvec4<std::uint8_t>;
};

}// namespace pf::mc
#endif//OPENGL_TEMPLATE_SRC_WORLD_VOXEL_H
