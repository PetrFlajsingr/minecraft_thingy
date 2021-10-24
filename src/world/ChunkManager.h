//
// Created by xflajs00 on 23.10.2021.
//

#ifndef MINECRAFT_THINGY_SRC_WORLD_CHUNKMANAGER_H
#define MINECRAFT_THINGY_SRC_WORLD_CHUNKMANAGER_H

#include "Chunk.h"
#include <glm/vec3.hpp>
#include <noise/PerlinNoiseGenerator.h>
#include <pf_common/math/ViewFrustum.h>
#include <pf_common/parallel/Safe.h>
#include <pf_common/parallel/ThreadPool.h>
#include <utils/Random.h>
#include <vector>
#include <utils/Direction.h>

namespace pf::mc {

class ChunkManager {
 public:
  explicit ChunkManager(std::size_t chunkLimit, double renderDistance, double seed = generateSeed());
  virtual ~ChunkManager();

  void resetWithSeed(double seed);
  [[nodiscard]] double getSeed() const;

  void generateChunks(glm::vec3 cameraPosition);

  struct RayCastResult {
    glm::ivec3 coords;
    Direction face;
  };
  [[nodiscard]] std::optional<RayCastResult> castRay(glm::vec3 position, glm::vec3 direction, double maxDistance = 10) const;

  [[nodiscard]] std::optional<Voxel> getVoxel(glm::ivec3 coords) const;
  void setVoxel(glm::ivec3 coords, Voxel::Type type);

  [[nodiscard]] std::vector<Chunk *> getChunksToRender(const math::ViewFrustum &viewFrustum, bool onlyFullyContained = false);

 private:
  void unloadDistantChunks(glm::vec3 cameraPosition);

  [[nodiscard]] Chunk *chunkForCoords(glm::ivec3 coords) const;

  [[nodiscard]] std::vector<glm::ivec3> getAllChunksToGenerate(glm::vec3 cameraPosition) const;

  Safe<std::vector<std::unique_ptr<Chunk>>> chunks;
  Safe<std::vector<glm::ivec3>> emptyChunks;
  std::size_t chunkLimit;
  double renderDistance;
  double seed;
  PerlinNoiseGenerator noiseGenerator;

  ThreadPool loadingThreadPool{4};
};

}// namespace pf::mc
#endif//MINECRAFT_THINGY_SRC_WORLD_CHUNKMANAGER_H
