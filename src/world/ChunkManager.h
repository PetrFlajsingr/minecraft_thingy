//
// Created by xflajs00 on 23.10.2021.
//

#ifndef MINECRAFT_THINGY_SRC_WORLD_CHUNKMANAGER_H
#define MINECRAFT_THINGY_SRC_WORLD_CHUNKMANAGER_H

#include <glm/vec3.hpp>
#include <vector>
#include "Chunk.h"
#include <pf_common/math/ViewFrustum.h>

namespace pf::mc {

class ChunkManager {
 public:
  explicit ChunkManager(std::size_t chunkLimit, double renderDistance);

  void generateChunks(glm::vec3 cameraPosition);

  std::vector<Chunk *> getChunksToRender(const math::ViewFrustum &viewFrustum);
 private:
  void unloadDistantChunks(glm::vec3 cameraPosition);

  std::vector<std::unique_ptr<Chunk>> chunks;
  std::size_t chunkLimit;
  double renderDistance;
};

}
#endif//MINECRAFT_THINGY_SRC_WORLD_CHUNKMANAGER_H
