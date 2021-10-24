//
// Created by xflajs00 on 23.10.2021.
//

#include "ChunkManager.h"
#include <algorithm>

pf::mc::ChunkManager::ChunkManager(std::size_t chunkLimit, double renderDistance, double seed) : chunkLimit(chunkLimit),
                                                                                                 renderDistance(renderDistance),
                                                                                                 seed(seed),
                                                                                                 noiseGenerator(seed) {
}

void pf::mc::ChunkManager::resetWithSeed(double seed) {
  chunks.clear();
  ChunkManager::seed = seed;
  noiseGenerator.setSeed(seed);
}

void pf::mc::ChunkManager::generateChunks(glm::vec3 cameraPosition) {
  unloadDistantChunks(cameraPosition);
  const auto newChunkPositions = getAllChunksToGenerate(cameraPosition);
  for (const auto &position : newChunkPositions) {
    chunks.emplace_back(std::make_unique<Chunk>(position, noiseGenerator));
  }
}

std::vector<pf::mc::Chunk *> pf::mc::ChunkManager::getChunksToRender(const pf::math::ViewFrustum &viewFrustum, bool onlyFullyContained) {
  std::vector<pf::mc::Chunk *> result{};
  for (const auto &chunk : chunks) {
    if (onlyFullyContained && viewFrustum.contains(chunk->getAABB()) == math::RelativePosition::Inside) {
      result.emplace_back(chunk.get());
    }
    if (!onlyFullyContained && viewFrustum.contains(chunk->getAABB()) != math::RelativePosition::Outside) {
      result.emplace_back(chunk.get());
    }
  }
  return result;
}
void pf::mc::ChunkManager::unloadDistantChunks(glm::vec3 cameraPosition) {
  chunks.erase(std::ranges::begin(std::ranges::remove_if(chunks, [&](const auto &chunk) {
                 return glm::distance(chunk->getCenter(), cameraPosition) > renderDistance;
               })),
               std::ranges::end(chunks));
}

double pf::mc::ChunkManager::getSeed() const {
  return seed;
}

std::vector<glm::vec3> pf::mc::ChunkManager::getAllChunksToGenerate(glm::vec3 cameraPosition) const {
  std::vector<glm::vec3> result{};
  const auto min = glm::ivec3{cameraPosition - static_cast<float>(renderDistance)} % static_cast<int>(CHUNK_LEN);
  const auto max = glm::ivec3{cameraPosition + static_cast<float>(renderDistance)} % static_cast<int>(CHUNK_LEN);
  for (auto x = min.x; x < max.x; x += CHUNK_LEN) {
    for (auto y = min.y; x < max.y; y += CHUNK_LEN) {
      for (auto z = min.z; x < max.z; z += CHUNK_LEN) {
        const auto pos = glm::vec3{x, y, z};
        result.emplace_back(pos);
      }
    }
  }
  result.erase(
      std::ranges::begin(std::ranges::remove_if(result, [this](const auto &pos) {
        for (const auto &chunk : chunks) {
          if (chunk->getPosition() == pos) {
            return true;
          }
        }
        return false;
      })),
      std::ranges::end(result));
  return result;
}
