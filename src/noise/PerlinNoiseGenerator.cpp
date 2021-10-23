//
// Created by xflajs00 on 23.10.2021.
//

#include "PerlinNoiseGenerator.h"

void pf::mc::PerlinNoiseGenerator::setSeed(std::uint32_t seed) {
  generator = siv::PerlinNoise{seed};
}

double pf::mc::PerlinNoiseGenerator::noise(glm::vec3 coord) const {
  return generator.accumulatedOctaveNoise3D(coord.x / 100.0, coord.y / 100.0, coord.z / 100.0, 3);
}
