//
// Created by xflajs00 on 23.10.2021.
//

#include "Chunk.h"
#include <fmt/core.h>
#include <log.h>

pf::mc::Chunk::Chunk(glm::vec3 position, const NoiseGenerator &noiseGenerator) : changed(true),
                                                                                 vbo(std::make_shared<Buffer>()),
                                                                                 vao(std::make_shared<VertexArray>()),
                                                                                 position(position) {
  generateVoxelData(noiseGenerator);
}

void pf::mc::Chunk::update() {
  if (!changed) {
    return;
  }
  log("updating chunk at {}x{}x{}", position.x, position.y, position.z);
  changed = false;
  const auto vertices = generateVertices();
  vertexCount = vertices.size();
  log("generated {} vertices", vertexCount);
  if (vertexCount == 0) {
    return;
  }
  vbo->alloc(vertexCount * sizeof(Voxel::Vertex), vertices.data(), GL_STATIC_DRAW);
  vao->addAttrib(vbo, 0, 4, GL_BYTE, 4 * sizeof(std::uint8_t));
}

void pf::mc::Chunk::render() {
  update();
  if (vertexCount == 0) {
    return;
  }
  vao->bind();

  glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

std::size_t pf::mc::Chunk::index3Dto1D(std::size_t x, std::size_t y, std::size_t z) {
  return z * CHUNK_LEN * CHUNK_LEN + y * CHUNK_LEN + x;
}

pf::mc::Voxel pf::mc::Chunk::getVoxel(std::size_t x, std::size_t y, std::size_t z) const {
  assert(x < CHUNK_LEN && y < CHUNK_LEN && z < CHUNK_LEN);
  const auto voxelIndex = index3Dto1D(x, y, z);
  return voxels[voxelIndex];
}

void pf::mc::Chunk::setVoxel(std::size_t x, std::size_t y, std::size_t z, pf::mc::Voxel::Type type) {
  assert(x < CHUNK_LEN && y < CHUNK_LEN && z < CHUNK_LEN);
  const auto voxelIndex = index3Dto1D(x, y, z);
  voxels[voxelIndex].type = type;
}

void pf::mc::Chunk::generateVoxelData(const pf::mc::NoiseGenerator &noiseGenerator) {
  for (std::size_t x = 0; x < CHUNK_LEN; ++x) {
    for (std::size_t y = 0; y < CHUNK_LEN; ++y) {
      for (std::size_t z = 0; z < CHUNK_LEN; ++z) {
        const auto index = index3Dto1D(x, y, z);
        const auto noiseValue = noiseGenerator.noise(position + glm::vec3{x, y, z});
        voxels[index].type = noiseValue > 0.0 ? Voxel::Type::Gravel : Voxel::Type::Empty;
      }
    }
  }
}

std::vector<pf::mc::Voxel::Vertex> pf::mc::Chunk::generateVertices() const {
  auto result = std::vector<Voxel::Vertex>{};
  result.reserve(CHUNK_SIZE * 3);// kinda random number
  // TODO: change this to single index
  for (std::size_t x = 0; x < CHUNK_LEN; ++x) {
    for (std::size_t y = 0; y < CHUNK_LEN; ++y) {
      for (std::size_t z = 0; z < CHUNK_LEN; ++z) {
        const auto index = index3Dto1D(x, y, z);
        const auto voxel = voxels[index];
        if (voxel.type == Voxel::Type::Empty) {
          continue;
        }
        // -X
        if (x == 0 || !isVoxelFilled(x - 1, y, z)) {
          result.emplace_back(x, y, z, voxel.type);
          result.emplace_back(x, y, z + 1, voxel.type);
          result.emplace_back(x, y + 1, z, voxel.type);
          result.emplace_back(x, y + 1, z, voxel.type);
          result.emplace_back(x, y, z + 1, voxel.type);
          result.emplace_back(x, y + 1, z + 1, voxel.type);
        }
        // +x
        if (x == CHUNK_LEN - 1 || !isVoxelFilled(x + 1, y, z)) {
          result.emplace_back(x + 1, y, z, voxel.type);
          result.emplace_back(x + 1, y + 1, z, voxel.type);
          result.emplace_back(x + 1, y, z + 1, voxel.type);
          result.emplace_back(x + 1, y + 1, z, voxel.type);
          result.emplace_back(x + 1, y + 1, z + 1, voxel.type);
          result.emplace_back(x + 1, y, z + 1, voxel.type);
        }
        // -y
        if (y == 0 || !isVoxelFilled(x, y - 1, z)) {
          result.emplace_back(x, y, z, voxel.type);
          result.emplace_back(x + 1, y, z, voxel.type);
          result.emplace_back(x, y, z + 1, voxel.type);
          result.emplace_back(x + 1, y, z, voxel.type);
          result.emplace_back(x + 1, y, z + 1, voxel.type);
          result.emplace_back(x, y, z + 1, voxel.type);
        }
        // +y
        if (y == CHUNK_LEN - 1 || !isVoxelFilled(x, y + 1, z)) {
          result.emplace_back(x, y + 1, z, voxel.type);
          result.emplace_back(x, y + 1, z + 1, voxel.type);
          result.emplace_back(x + 1, y + 1, z, voxel.type);
          result.emplace_back(x + 1, y + 1, z, voxel.type);
          result.emplace_back(x, y + 1, z + 1, voxel.type);
          result.emplace_back(x + 1, y + 1, z + 1, voxel.type);
        }
        // -z
        if (z == 0 || !isVoxelFilled(x, y, z - 1)) {
          result.emplace_back(x, y, z, voxel.type);
          result.emplace_back(x, y + 1, z, voxel.type);
          result.emplace_back(x + 1, y, z, voxel.type);
          result.emplace_back(x, y + 1, z, voxel.type);
          result.emplace_back(x + 1, y + 1, z, voxel.type);
          result.emplace_back(x + 1, y, z, voxel.type);
        }
        // +z
        if (z == CHUNK_LEN - 1 || !isVoxelFilled(x, y, z + 1)) {
          result.emplace_back(x, y, z + 1, voxel.type);
          result.emplace_back(x + 1, y, z + 1, voxel.type);
          result.emplace_back(x, y + 1, z + 1, voxel.type);
          result.emplace_back(x, y + 1, z + 1, voxel.type);
          result.emplace_back(x + 1, y, z + 1, voxel.type);
          result.emplace_back(x + 1, y + 1, z + 1, voxel.type);
        }
      }
    }
  }
  return result;
}

bool pf::mc::Chunk::isVoxelFilled(std::size_t x, std::size_t y, std::size_t z) const {
  return isVoxelFilled(index3Dto1D(x, y, z));
}

bool pf::mc::Chunk::isVoxelFilled(std::size_t index) const {
  return voxels[index].type != Voxel::Type::Empty;
}
void pf::mc::Chunk::setChanged() {
  changed = true;
}
