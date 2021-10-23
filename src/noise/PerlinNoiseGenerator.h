//
// Created by xflajs00 on 23.10.2021.
//

#ifndef MINECRAFT_THINGY_SRC_NOISE_PERLINNOISEGENERATOR_H
#define MINECRAFT_THINGY_SRC_NOISE_PERLINNOISEGENERATOR_H

#include "NoiseGenerator.h"
#include <PerlinNoise/PerlinNoise.hpp>

namespace pf::mc {
class PerlinNoiseGenerator : public NoiseGenerator {
 public:
  void setSeed(std::uint32_t seed) override;
  [[nodiscard]] double noise(glm::vec3 coord) const override;

 private:
  siv::PerlinNoise generator;
};
}

#endif//MINECRAFT_THINGY_SRC_NOISE_PERLINNOISEGENERATOR_H