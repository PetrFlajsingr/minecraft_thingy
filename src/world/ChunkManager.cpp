//
// Created by xflajs00 on 23.10.2021.
//

#include "ChunkManager.h"
#include <algorithm>

pf::mc::ChunkManager::ChunkManager(std::size_t chunkLimit, double renderDistance) : chunkLimit(chunkLimit), renderDistance(renderDistance) {
}

void pf::mc::ChunkManager::generateChunks(glm::vec3 cameraPosition) {
}

std::vector<pf::mc::Chunk *> pf::mc::ChunkManager::getChunksToRender(const pf::math::ViewFrustum &viewFrustum) {
  return std::vector<Chunk *>();
}
void pf::mc::ChunkManager::unloadDistantChunks(glm::vec3 cameraPosition) {
  chunks.erase(std::ranges::begin(std::ranges::remove_if(chunks.begin(), chunks.end(), [&](const auto &chunk) {
                 return glm::distance(chunk->getCenter(), cameraPosition) > renderDistance;
               })),
               std::ranges::end(chunks));
}
