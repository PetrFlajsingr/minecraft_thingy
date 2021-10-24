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
#include <pf_common/parallel/ThreadPool.h>
#include <pf_common/parallel/Safe.h>

namespace pf::mc {

class ChunkManager {
 public:
  explicit ChunkManager(std::size_t chunkLimit, double renderDistance, double seed = generateSeed());
  virtual ~ChunkManager();

  void resetWithSeed(double seed);
  [[nodiscard]] double getSeed() const;

  void generateChunks(glm::vec3 cameraPosition);

  [[nodiscard]] std::vector<Chunk *> getChunksToRender(const math::ViewFrustum &viewFrustum, bool onlyFullyContained = false);
 private:
  void unloadDistantChunks(glm::vec3 cameraPosition);

  [[nodiscard]] std::vector<glm::ivec3> getAllChunksToGenerate(glm::vec3 cameraPosition) const;

  Safe<std::vector<std::unique_ptr<Chunk>>> chunks;
  Safe<std::vector<glm::ivec3>> emptyChunks;
  std::size_t chunkLimit;
  double renderDistance;
  double seed;
  PerlinNoiseGenerator noiseGenerator;

  ThreadPool loadingThreadPool{4};
};

}
#endif//MINECRAFT_THINGY_SRC_WORLD_CHUNKMANAGER_H
