//
// Created by xflajs00 on 23.10.2021.
//

#ifndef MINECRAFT_THINGY_SRC_WORLD_CHUNKMANAGER_H
#define MINECRAFT_THINGY_SRC_WORLD_CHUNKMANAGER_H

#include <glm/vec3.hpp>
#include <vector>
#include "Chunk.h"
#include <pf_common/math/ViewFrustum.h>
#include <utils/Random.h>
#include <noise/PerlinNoiseGenerator.h>

namespace pf::mc {

class ChunkManager {
 public:
  explicit ChunkManager(std::size_t chunkLimit, double renderDistance, double seed = generateSeed());

  void resetWithSeed(double seed);
  [[nodiscard]] double getSeed() const;

  void generateChunks(glm::vec3 cameraPosition);

  [[nodiscard]] std::vector<Chunk *> getChunksToRender(const math::ViewFrustum &viewFrustum, bool onlyFullyContained = false);
 private:
  void unloadDistantChunks(glm::vec3 cameraPosition);

  [[nodiscard]] std::vector<glm::vec3> getAllChunksToGenerate(glm::vec3 cameraPosition) const;

  std::vector<std::unique_ptr<Chunk>> chunks;
  std::size_t chunkLimit;
  double renderDistance;
  double seed;
  PerlinNoiseGenerator noiseGenerator;
};

}
#endif//MINECRAFT_THINGY_SRC_WORLD_CHUNKMANAGER_H
