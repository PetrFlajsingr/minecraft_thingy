//
// Created by xflajs00 on 23.10.2021.
//

#ifndef MINECRAFT_THINGY_SRC_NOISE_NOISEGENERATOR_H
#define MINECRAFT_THINGY_SRC_NOISE_NOISEGENERATOR_H

#include <glm/vec3.hpp>

namespace pf::mc {
class NoiseGenerator {
 public:
  virtual ~NoiseGenerator() = default;

  virtual void setSeed(std::uint32_t seed) = 0;
  [[nodiscard]] virtual double noise(glm::vec3 coord) const = 0;
};
}

#endif//MINECRAFT_THINGY_SRC_NOISE_NOISEGENERATOR_H
