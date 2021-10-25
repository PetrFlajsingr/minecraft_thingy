//
// Created by xflajs00 on 23.10.2021.
//

#include "ChunkManager.h"
#include <algorithm>
#include <log.h>
#include <pf_common/bin.h>
#include <utility>
#include <log.h>

pf::mc::ChunkManager::ChunkManager(std::size_t chunkLimit, double renderDistance, double seed) : chunkLimit(chunkLimit),
                                                                                                 renderDistance(renderDistance),
                                                                                                 seed(seed),
                                                                                                 noiseGenerator(std::make_shared<PerlinNoiseGenerator>(seed)) {
}

pf::mc::ChunkManager::~ChunkManager() {
  loadingThreadPool.cancelAndStop();
}

void pf::mc::ChunkManager::resetWithSeed(double seed) {
  chunks.writeAccess()->clear();
  emptyChunks.writeAccess()->clear();
  hiddenChunkChanges.clear();
  ChunkManager::seed = seed;
  noiseGenerator->setSeed(seed);
  isFirstGen = true;
}

void pf::mc::ChunkManager::generateChunks(glm::vec3 cameraPosition) {
  unloadDistantChunks(cameraPosition);
  const auto newChunkPositions = getAllChunksToGenerate(cameraPosition);

  for (const auto &position : newChunkPositions) {
    auto emptyChunksAccess = emptyChunks.readOnlyAccess();
    if (std::ranges::find(*emptyChunksAccess, position) != std::ranges::end(*emptyChunksAccess)) {
      continue;
    }
    if (auto iter = std::ranges::find_if(hiddenChunkChanges, [position](const auto &change) { return change.position == position; });
        iter != hiddenChunkChanges.end()) {
      auto newChunk = std::make_unique<Chunk>(position, noiseGenerator);
      newChunk->setChanges(iter->changes);
      chunks.writeAccess()->emplace_back(std::move(newChunk));
      hiddenChunkChanges.erase(iter);
      continue;
    }
    // creating the chunk outside thread because it creates buffers
    auto newChunk = new Chunk(position, noiseGenerator);// FIXME only using new here 'cause no time and MSVC template errors really suck
    loadingThreadPool.enqueue([position, this, newChunk] {
      if (chunks.readOnlyAccess()->size() >= chunkLimit) { return; }
      newChunk->createMesh();
      if (newChunk->getVertexCount() == 0) {
        delete newChunk;// FIXME only using delete here 'cause no time and MSVC template errors really suck
        emptyChunks.writeAccess()->emplace_back(position);
      } else {
        chunks.writeAccess()->emplace_back(std::unique_ptr<Chunk>(newChunk));
      }
    });
  }
}

std::optional<pf::mc::Voxel> pf::mc::ChunkManager::getVoxel(glm::ivec3 coords) const {
  const auto chunk = chunkForCoords(coords);
  if (chunk == nullptr) {
    return std::nullopt;
  }
  const auto localCoords = coords - chunk->getPosition();
  return chunk->getVoxel(localCoords);
}

void pf::mc::ChunkManager::setVoxel(glm::ivec3 coords, pf::mc::Voxel::Type type) {
  auto chunk = chunkForCoords(coords);
  if (chunk == nullptr) {
    log("No chunk found");
    if (type == Voxel::Type::Empty) {
      return;
    }
    const auto chunkCoords = coords - glm::abs(coords % static_cast<int>(CHUNK_LEN));
    auto emptyChunksAccess = emptyChunks.writeAccess();
    std::unique_ptr<Chunk> newChunk = nullptr;
    if (const auto iter = std::ranges::find_if(*emptyChunksAccess, [chunkCoords](auto coord) { return coord == chunkCoords; }); iter != emptyChunksAccess->end()) {
      emptyChunksAccess->erase(iter);
      newChunk = std::make_unique<Chunk>(chunkCoords, noiseGenerator);
    } else {
      newChunk = std::make_unique<Chunk>(chunkCoords, noiseGenerator);
      newChunk->createMesh();
    }
    chunk = chunks.writeAccess()->emplace_back(std::move(newChunk)).get();
  }
  const auto localCoords = coords - chunk->getPosition();
  chunk->setVoxel(localCoords, type);
}

std::vector<pf::mc::Chunk *> pf::mc::ChunkManager::getChunksToRender(const pf::math::ViewFrustum &viewFrustum, bool onlyFullyContained) {
  std::vector<pf::mc::Chunk *> result{};
  for (const auto &chunk : *chunks.readOnlyAccess()) {
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
  const auto shouldUnload = [&](const auto &chunk) {
    return glm::distance(chunk->getCenter(), cameraPosition) > renderDistance;
  };
  auto chunksAccess = chunks.writeAccess();
  for (auto &chunk : *chunksAccess) {
    if (shouldUnload(chunk) && chunk->isModified()) {
      hiddenChunkChanges.emplace_back(chunk->getPosition(), chunk->getChanges());
    }
  }
  auto toRemove = std::ranges::remove_if(*chunksAccess, [&](const auto &chunk) {
    return chunk == nullptr || shouldUnload(chunk);
  });
  if (!toRemove.empty()) {
    log("Unloading {} chunks", toRemove.size());
  }
  chunksAccess->erase(std::ranges::begin(toRemove),
                      std::ranges::end(*chunksAccess));
}

double pf::mc::ChunkManager::getSeed() const {
  return seed;
}

std::vector<glm::ivec3> pf::mc::ChunkManager::getAllChunksToGenerate(glm::vec3 cameraPosition) {
  std::vector<glm::ivec3> result{};
  previousPosition = cameraPosition;

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
        return std::ranges::any_of(*chunks.readOnlyAccess(), [pos](const auto &chunk) { return chunk->getPosition() == pos; });
      })),
      std::ranges::end(result));
  std::ranges::sort(result, [cameraPosition](const auto &lhs, const auto &rhs) {
    return glm::distance(cameraPosition, glm::vec3{lhs}) < glm::distance(cameraPosition, glm::vec3{rhs});
  });
  return result;
}

pf::mc::Chunk *pf::mc::ChunkManager::chunkForCoords(glm::ivec3 coords) const {
  for (const auto &chunk : *chunks.readOnlyAccess()) {
    const auto diff = coords - chunk->getPosition();
    if (diff.x < CHUNK_LEN && diff.y < CHUNK_LEN && diff.z < CHUNK_LEN) {
      return chunk.get();
    }
  }
  return nullptr;
}

std::optional<pf::mc::ChunkManager::RayCastResult> pf::mc::ChunkManager::castRay(glm::vec3 position, glm::vec3 direction, double maxDistance) const {
  constexpr auto RAY_STEP = 0.01f;
  RayCastResult result{};
  glm::ivec3 rayPosition = position;
  glm::ivec3 previousPosition{};
  for (float i = 0; i <= maxDistance; i += RAY_STEP) {
    previousPosition = rayPosition;
    rayPosition = position + direction * i;
    if (const auto voxel = getVoxel(rayPosition); voxel.has_value() && voxel->type != Voxel::Type::Empty) {
      result.coords = rayPosition;
      if (previousPosition.x > rayPosition.x) {
        result.face = Direction::Right;
      } else if (previousPosition.x < rayPosition.x) {
        result.face = Direction::Left;
      } else if (previousPosition.y > rayPosition.y) {
        result.face = Direction::Up;
      } else if (previousPosition.y < rayPosition.y) {
        result.face = Direction::Down;
      } else if (previousPosition.z > rayPosition.z) {
        result.face = Direction::Backward;
      } else if (previousPosition.z < rayPosition.z) {
        result.face = Direction::Forward;// might need to switch these up
      }
      return result;
    }
  }
  return std::nullopt;
}

nlohmann::json pf::mc::ChunkManager::serialize() const {
  nlohmann::json result;
  result["seed"] = seed;
  result["chunks"] = nlohmann::json::array();

  std::ranges::for_each(hiddenChunkChanges, [&](const auto &change) {
    auto chunk = Chunk{change.position, noiseGenerator};
    chunk.setChanges(change.changes);
    const auto serializedChunk = chunk.serialize();
    result["chunks"].emplace_back(serializedChunk);
  });
  std::ranges::for_each(*chunks.readOnlyAccess(), [&](const auto &chunk) {
    if (chunk->isModified()) {
      const auto serializedChunk = chunk->serialize();
      result["chunks"].emplace_back(serializedChunk);
    }
  });
  return result;
}

void pf::mc::ChunkManager::resetAndDeserialize(const nlohmann::json &data) {
  const double newSeed = data["seed"];
  resetWithSeed(newSeed);

  auto chunksAccess = chunks.writeAccess();
  for (const auto &chunkInfo : data["chunks"]) {
    auto newChunk = std::make_unique<Chunk>(noiseGenerator, chunkInfo);
    chunksAccess->emplace_back(std::move(newChunk));
  }
}

pf::mc::ChunkManager::ChunkChangeData::ChunkChangeData(const glm::ivec3 &position, pf::mc::Chunk::ChangeStorage changes) : position(position), changes(std::move(changes)) {}
