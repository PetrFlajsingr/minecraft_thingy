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

  [[nodiscard]] std::vector<std::byte> serialize() const;

  void resetAndDeserialize(const std::vector<std::byte> &data);

 private:
  void unloadDistantChunks(glm::vec3 cameraPosition);

  [[nodiscard]] Chunk *chunkForCoords(glm::ivec3 coords) const;

  [[nodiscard]] std::vector<glm::ivec3> getAllChunksToGenerate(glm::vec3 cameraPosition);

  Safe<std::vector<std::unique_ptr<Chunk>>> chunks;

  struct ChunkChangeData {
    ChunkChangeData(const glm::ivec3 &position, Chunk::ChangeStorage changes);
    glm::ivec3 position;
    Chunk::ChangeStorage changes;
  };
  std::vector<ChunkChangeData> hiddenChunkChanges;

  Safe<std::vector<glm::ivec3>> emptyChunks;
  std::size_t chunkLimit;
  double renderDistance;
  double seed;
  std::shared_ptr<NoiseGenerator> noiseGenerator;

  ThreadPool loadingThreadPool{4};

  bool isFirstGen = true;
  glm::vec3 previousPosition;
};

}// namespace pf::mc
#endif//MINECRAFT_THINGY_SRC_WORLD_CHUNKMANAGER_H
