//
// Created by xflajs00 on 23.10.2021.
//

#ifndef OPENGL_TEMPLATE_SRC_WORLD_VOXEL_H
#define OPENGL_TEMPLATE_SRC_WORLD_VOXEL_H

#include <cstdint>
#include <glm/vec4.hpp>

namespace pf::mc {

struct alignas(1) Voxel {
  enum class Type : std::uint8_t {
    Empty = 0, Gravel = 1, Dirt = 2, Grass = 3, Ice = 4
  };
  Type type;

  using Vertex = glm::tvec4<std::uint8_t>;
};

}// namespace pf::mc
#endif//OPENGL_TEMPLATE_SRC_WORLD_VOXEL_H
