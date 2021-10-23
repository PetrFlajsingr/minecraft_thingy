//
// Created by xflajs00 on 23.10.2021.
//

#include "Chunk.h"
#include <fmt/core.h>
#include <log.h>

pf::mc::Chunk::Chunk(glm::vec3 position, const NoiseGenerator &noiseGenerator) : changed(true),
                                                                                 vbo(std::make_shared<Buffer>()),
                                                                                 nbo(std::make_shared<Buffer>()),
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
  const auto data = generateGeometry();
  vertexCount = data.vertices.size();
  log("generated {} vertices", vertexCount);
  if (vertexCount == 0) {
    return;
  }
  vbo->alloc(vertexCount * sizeof(Voxel::Vertex), data.vertices.data(), GL_STATIC_DRAW);
  nbo->alloc(data.normals.size() * sizeof(glm::vec3), data.normals.data(), GL_STATIC_DRAW);
  vao->addAttrib(vbo, 0, 4, GL_BYTE, 4 * sizeof(std::uint8_t));
  vao->addAttrib(nbo, 1, 3, GL_FLOAT, 3 * sizeof(float));
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
  constexpr auto HEIGHT_GRAVEL = 5;
  constexpr auto HEIGHT_ICE = 8;
  for (std::size_t x = 0; x < CHUNK_LEN; ++x) {
    for (std::size_t y = 0; y < CHUNK_LEN; ++y) {
      for (std::size_t z = 0; z < CHUNK_LEN; ++z) {
        const auto index = index3Dto1D(x, y, z);
        const auto noiseValue = noiseGenerator.noise(position + glm::vec3{x, y, z});
        voxels[index].type = noiseValue > 0.0 ? Voxel::Type::Gravel : Voxel::Type::Empty;
      }
    }
  }
  for (std::size_t x = 0; x < CHUNK_LEN; ++x) {
    for (std::size_t y = 0; y < CHUNK_LEN; ++y) {
      for (std::size_t z = 0; z < CHUNK_LEN; ++z) {
        const auto index = index3Dto1D(x, y, z);
        if (voxels[index].type != Voxel::Type::Empty) {
          if (y < HEIGHT_GRAVEL) {
            voxels[index].type = Voxel::Type::Gravel;
            continue;
          }
          if (y < CHUNK_LEN - 1 && !isVoxelFilled(x, y+1, z)) {
            if (y > HEIGHT_ICE) {
              voxels[index].type = Voxel::Type::Gravel;
            } else {
              voxels[index].type = Voxel::Type::Grass;
            }
            continue;
          }
          voxels[index].type = Voxel::Type::Dirt;
        }
      }
    }
  }
}

pf::mc::Chunk::GeometryData pf::mc::Chunk::generateGeometry() const {
  auto result = GeometryData{};
  result.vertices.reserve(CHUNK_SIZE * 3);// kinda random number
  result.normals.reserve(CHUNK_SIZE);// kinda random number
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
          result.vertices.emplace_back(x, y, z, voxel.type);
          result.normals.emplace_back(-1, 0, 0);
          result.vertices.emplace_back(x, y, z + 1, voxel.type);
          result.normals.emplace_back(-1, 0, 0);
          result.vertices.emplace_back(x, y + 1, z, voxel.type);
          result.normals.emplace_back(-1, 0, 0);
          result.vertices.emplace_back(x, y + 1, z, voxel.type);
          result.normals.emplace_back(-1, 0, 0);
          result.vertices.emplace_back(x, y, z + 1, voxel.type);
          result.normals.emplace_back(-1, 0, 0);
          result.vertices.emplace_back(x, y + 1, z + 1, voxel.type);
          result.normals.emplace_back(-1, 0, 0);
        }
        // +x
        if (x == CHUNK_LEN - 1 || !isVoxelFilled(x + 1, y, z)) {
          result.vertices.emplace_back(x + 1, y, z, voxel.type);
          result.normals.emplace_back(1, 0, 0);
          result.vertices.emplace_back(x + 1, y + 1, z, voxel.type);
          result.normals.emplace_back(1, 0, 0);
          result.vertices.emplace_back(x + 1, y, z + 1, voxel.type);
          result.normals.emplace_back(1, 0, 0);
          result.vertices.emplace_back(x + 1, y + 1, z, voxel.type);
          result.normals.emplace_back(1, 0, 0);
          result.vertices.emplace_back(x + 1, y + 1, z + 1, voxel.type);
          result.normals.emplace_back(1, 0, 0);
          result.vertices.emplace_back(x + 1, y, z + 1, voxel.type);
          result.normals.emplace_back(1, 0, 0);
        }
        // -y
        if (y == 0 || !isVoxelFilled(x, y - 1, z)) {
          result.vertices.emplace_back(x, y, z, voxel.type);
          result.normals.emplace_back(0, -1, 0);
          result.vertices.emplace_back(x + 1, y, z, voxel.type);
          result.normals.emplace_back(0, -1, 0);
          result.vertices.emplace_back(x, y, z + 1, voxel.type);
          result.normals.emplace_back(0, -1, 0);
          result.vertices.emplace_back(x + 1, y, z, voxel.type);
          result.normals.emplace_back(0, -1, 0);
          result.vertices.emplace_back(x + 1, y, z + 1, voxel.type);
          result.normals.emplace_back(0, -1, 0);
          result.vertices.emplace_back(x, y, z + 1, voxel.type);
          result.normals.emplace_back(0, -1, 0);
        }
        // +y
        if (y == CHUNK_LEN - 1 || !isVoxelFilled(x, y + 1, z)) {
          result.vertices.emplace_back(x, y + 1, z, voxel.type);
          result.normals.emplace_back(0, 1, 0);
          result.vertices.emplace_back(x, y + 1, z + 1, voxel.type);
          result.normals.emplace_back(0, 1, 0);
          result.vertices.emplace_back(x + 1, y + 1, z, voxel.type);
          result.normals.emplace_back(0, 1, 0);
          result.vertices.emplace_back(x + 1, y + 1, z, voxel.type);
          result.normals.emplace_back(0, 1, 0);
          result.vertices.emplace_back(x, y + 1, z + 1, voxel.type);
          result.normals.emplace_back(0, 1, 0);
          result.vertices.emplace_back(x + 1, y + 1, z + 1, voxel.type);
          result.normals.emplace_back(0, 1, 0);
        }
        // -z
        if (z == 0 || !isVoxelFilled(x, y, z - 1)) {
          result.vertices.emplace_back(x, y, z, voxel.type);
          result.normals.emplace_back(0, 0, -1);
          result.vertices.emplace_back(x, y + 1, z, voxel.type);
          result.normals.emplace_back(0, 0, -1);
          result.vertices.emplace_back(x + 1, y, z, voxel.type);
          result.normals.emplace_back(0, 0, -1);
          result.vertices.emplace_back(x, y + 1, z, voxel.type);
          result.normals.emplace_back(0, 0, -1);
          result.vertices.emplace_back(x + 1, y + 1, z, voxel.type);
          result.normals.emplace_back(0, 0, -1);
          result.vertices.emplace_back(x + 1, y, z, voxel.type);
          result.normals.emplace_back(0, 0, -1);
        }
        // +z
        if (z == CHUNK_LEN - 1 || !isVoxelFilled(x, y, z + 1)) {
          result.vertices.emplace_back(x, y, z + 1, voxel.type);
          result.normals.emplace_back(0, 0, 1);
          result.vertices.emplace_back(x + 1, y, z + 1, voxel.type);
          result.normals.emplace_back(0, 0, 1);
          result.vertices.emplace_back(x, y + 1, z + 1, voxel.type);
          result.normals.emplace_back(0, 0, 1);
          result.vertices.emplace_back(x, y + 1, z + 1, voxel.type);
          result.normals.emplace_back(0, 0, 1);
          result.vertices.emplace_back(x + 1, y, z + 1, voxel.type);
          result.normals.emplace_back(0, 0, 1);
          result.vertices.emplace_back(x + 1, y + 1, z + 1, voxel.type);
          result.normals.emplace_back(0, 0, 1);
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
