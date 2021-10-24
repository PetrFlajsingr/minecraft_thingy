//
// Created by xflajs00 on 23.10.2021.
//

#include "ChunkManager.h"
#include <algorithm>
#include <log.h>

pf::mc::ChunkManager::ChunkManager(std::size_t chunkLimit, double renderDistance, double seed) : chunkLimit(chunkLimit),
                                                                                                 renderDistance(renderDistance),
                                                                                                 seed(seed),
                                                                                                 noiseGenerator(seed) {
}

pf::mc::ChunkManager::~ChunkManager() {
  loadingThreadPool.cancelAndStop();
}

void pf::mc::ChunkManager::resetWithSeed(double seed) {
  chunks.clear();
  emptyChunks.clear();
  ChunkManager::seed = seed;
  noiseGenerator.setSeed(seed);
}

void pf::mc::ChunkManager::generateChunks(glm::vec3 cameraPosition) {
  unloadDistantChunks(cameraPosition);
  const auto newChunkPositions = getAllChunksToGenerate(cameraPosition);

  using ChunkResultPair = std::pair<Chunk *, glm::ivec3>;
  using FutureReturnType = std::future<ChunkResultPair>;
  std::vector<FutureReturnType> futures{};
  for (const auto &position : newChunkPositions) {
    if (std::ranges::find(emptyChunks, position) != std::ranges::end(emptyChunks)) {
      continue;
    }
    if (chunks.size() >= chunkLimit) { break; }
    // creating the chunk outside thread because it creates buffers
    auto newChunk = new Chunk(position, noiseGenerator);// FIXME only using new here 'cause no time and MSVC template errors really suck
    futures.emplace_back(
        loadingThreadPool.enqueue([position, this, newChunk]() -> ChunkResultPair {
          newChunk->createMesh();
          if (newChunk->getVertexCount() == 0) {
            delete newChunk;// FIXME only using delete here 'cause no time and MSVC template errors really suck
            return {nullptr, position};
          }
          return {newChunk, position};
        }));
  }
  for (auto &future : futures) {
    const auto &[newChunk, position] = std::move(future.get());
    if (newChunk == nullptr) {
      emptyChunks.emplace_back(position);
      delete newChunk;// FIXME only using delete here 'cause no time and MSVC template errors really suck
    } else {
      chunks.push_back(std::unique_ptr<Chunk>(newChunk));
    }
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
  auto toRemove = std::ranges::remove_if(chunks, [&](const auto &chunk) {
    return glm::distance(chunk->getCenter(), cameraPosition) > renderDistance;
  });
  if (!toRemove.empty()) {
    log("Unloading {} chunks", toRemove.size());
  }
  chunks.erase(std::ranges::begin(toRemove),
               std::ranges::end(chunks));
}

double pf::mc::ChunkManager::getSeed() const {
  return seed;
}

std::vector<glm::ivec3> pf::mc::ChunkManager::getAllChunksToGenerate(glm::vec3 cameraPosition) const {
  std::vector<glm::ivec3> result{};
  auto min = glm::ivec3{cameraPosition - static_cast<float>(renderDistance)};
  min -= min % static_cast<int>(CHUNK_LEN);
  auto max = glm::ivec3{cameraPosition + static_cast<float>(renderDistance)};
  max -= max % static_cast<int>(CHUNK_LEN);
  for (auto x = min.x; x < max.x; x += CHUNK_LEN) {
    for (auto y = min.y; y < max.y; y += CHUNK_LEN) {
      for (auto z = min.z; z < max.z; z += CHUNK_LEN) {
        const auto pos = glm::ivec3{x, y, z};
        const auto center = glm::vec3{pos} + static_cast<float>(CHUNK_LEN) / 2;
        if (glm::distance(center, cameraPosition) > renderDistance) { continue; }
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
  std::ranges::sort(result, [cameraPosition](const auto &lhs, const auto &rhs) {
    return glm::distance(cameraPosition, glm::vec3{lhs}) < glm::distance(cameraPosition, glm::vec3{rhs});
  });
  return result;
}