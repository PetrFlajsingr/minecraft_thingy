//
// Created by xflajs00 on 23.10.2021.
//

#ifndef MINECRAFT_THINGY_SRC_NOISE_PERLINNOISEGENERATOR_H
#define MINECRAFT_THINGY_SRC_NOISE_PERLINNOISEGENERATOR_H

#include "NoiseGenerator.h"

namespace pf::mc {
class PerlinNoiseGenerator : public NoiseGenerator {
 public:
  PerlinNoiseGenerator();
  explicit PerlinNoiseGenerator(double seed);
  void setSeed(double seed) override;
  [[nodiscard]] double noise(glm::vec3 coord) const override;

 private:
  float seed;
};
}

#endif//MINECRAFT_THINGY_SRC_NOISE_PERLINNOISEGENERATOR_H
