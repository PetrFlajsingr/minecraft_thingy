//
// Created by xflajs00 on 23.10.2021.
//

#include "Chunk.h"
#include <fmt/core.h>
#include <log.h>
#include <pf_common/bin.h>

#include <utility>

pf::mc::Chunk::Chunk(glm::ivec3 position, std::shared_ptr<NoiseGenerator> noiseGenerator) : noiseGenerator(std::move(noiseGenerator)),
                                                                                            changed(true),
                                                                                            vbo(std::make_shared<Buffer>()),
                                                                                            nbo(std::make_shared<Buffer>()),
                                                                                            vao(std::make_shared<VertexArray>()),
                                                                                            position(position),
                                                                                            center(glm::vec3{position} + CHUNK_LEN / 2.0f) {
}

pf::mc::Chunk::Chunk(std::shared_ptr<NoiseGenerator> noiseGenerator,const nlohmann::json &data) : noiseGenerator(std::move(noiseGenerator)),
                                                                                                        changed(true),
                                                                                                        vbo(std::make_shared<Buffer>()),
                                                                                                        nbo(std::make_shared<Buffer>()),
                                                                                                        vao(std::make_shared<VertexArray>()) {
  const auto jsonPosition = data["position"];
  position.x = jsonPosition["x"];
  position.y = jsonPosition["y"];
  position.z = jsonPosition["z"];

  center = glm::vec3{position} + CHUNK_LEN / 2.0f;

  ChangeStorage newChanges;
  for (const auto &change : data["changes"]) {
    const auto jsonChangePosition = change["position"];
    LowResPoint pos;
    pos.x = jsonChangePosition["x"];
    pos.y = jsonChangePosition["y"];
    pos.z = jsonChangePosition["z"];
    const std::uint8_t typeAsInt = change["type"];
    const auto type = static_cast<Voxel::Type>(typeAsInt);
    newChanges[pos] = type;
  }
  generateVoxelData();

  setChanges(newChanges);
}

void pf::mc::Chunk::update() {
  if (!changed) {
    return;
  }
  changed = false;
  createMesh();
  if (vertexCount == 0) {
    return;
  }
  vbo->alloc(vertexCount * sizeof(Voxel::Vertex), mesh.vertices.data(), GL_STATIC_DRAW);
  nbo->alloc(mesh.normals.size() * sizeof(glm::vec3), mesh.normals.data(), GL_STATIC_DRAW);
  vao->addAttrib(vbo, 0, 4, GL_BYTE, 4 * sizeof(std::uint8_t));
  vao->addAttrib(nbo, 1, 3, GL_FLOAT, 3 * sizeof(float));
  mesh.normals.clear();
  mesh.vertices.clear();
  mesh.normals.shrink_to_fit();
  mesh.vertices.shrink_to_fit();
}

void pf::mc::Chunk::createMesh() {
  if (geometryGenerated) {
    return;
  }
  generateVoxelData();
  geometryGenerated = true;
  //log("updating chunk at {}x{}x{}", position.x, position.y, position.z); FIXME need to sync log with main thread

  mesh = generateGeometry();
  vertexCount = mesh.vertices.size();
  //log("generated {} vertices", vertexCount); FIXME need to sync log with main thread
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

pf::mc::Voxel pf::mc::Chunk::getVoxel(glm::ivec3 coords) const {
  assert(coords.x < CHUNK_LEN && coords.y < CHUNK_LEN && coords.z < CHUNK_LEN);
  const auto voxelIndex = index3Dto1D(coords.x, coords.y, coords.z);
  return voxels[voxelIndex];
}

void pf::mc::Chunk::setVoxel(glm::ivec3 coords, pf::mc::Voxel::Type type) {
  assert(coords.x < CHUNK_LEN && coords.y < CHUNK_LEN && coords.z < CHUNK_LEN);
  const auto voxelIndex = index3Dto1D(coords.x, coords.y, coords.z);
  voxels[voxelIndex].type = type;
  setChanged();
  modified = true;
  changes[LowResPoint{coords.x, coords.y, coords.z}] = type;
}

void pf::mc::Chunk::generateVoxelData() {
  if (voxelDataGenerated) {
    return;
  }
  voxelDataGenerated = true;
  constexpr auto HEIGHT_ICE = 30;
  for (std::size_t x = 0; x < CHUNK_LEN; ++x) {
    for (std::size_t y = 0; y < CHUNK_LEN; ++y) {
      for (std::size_t z = 0; z < CHUNK_LEN; ++z) {
        const auto index = index3Dto1D(x, y, z);
        const auto noiseValue = noiseGenerator->noise(glm::vec3{position} + glm::vec3{x, y, z});
        if (noiseValue < 0.0) {
          voxels[index].type = Voxel::Type::Empty;
        } else if (noiseValue > 14.0) {
          voxels[index].type = Voxel::Type::Gravel;
        } else {
          voxels[index].type = Voxel::Type::Dirt;
        }
      }
    }
  }
  for (std::size_t x = 0; x < CHUNK_LEN; ++x) {
    for (std::size_t y = 0; y < CHUNK_LEN; ++y) {
      for (std::size_t z = 0; z < CHUNK_LEN; ++z) {
        const auto index = index3Dto1D(x, y, z);
        if (voxels[index].type != Voxel::Type::Empty) {
          if (voxels[index].type == Voxel::Type::Gravel) { continue; }
          if (y < CHUNK_LEN - 1 && !isVoxelFilled(x, y + 1, z)) {
            if (y + position.y > HEIGHT_ICE) {
              voxels[index].type = Voxel::Type::Ice;
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
  result.normals.reserve(CHUNK_SIZE);     // kinda random number
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
  geometryGenerated = false;
}

const glm::ivec3 &pf::mc::Chunk::getPosition() const {
  return position;
}

const glm::vec3 &pf::mc::Chunk::getCenter() const {
  return center;
}

pf::math::BoundingBox<3> pf::mc::Chunk::getAABB() const {
  return {position, glm::vec3{position} + static_cast<float>(CHUNK_LEN)};
}

std::size_t pf::mc::Chunk::getVertexCount() const {
  return vertexCount;
}

bool pf::mc::Chunk::isModified() const {
  return modified;
}

nlohmann::json pf::mc::Chunk::serialize() const {
  nlohmann::json result;
  result["position"] = {
      {"x", position.x},
      {"y", position.y},
      {"z", position.z},
  };
  result["changes"] = nlohmann::json::array();
  std::ranges::for_each(changes, [&result](const auto &change) {
    const auto &[position, type] = change;
    nlohmann::json jsonChange = {
        {"position", {
                         {"x", position.x},
                         {"y", position.y},
                         {"z", position.z},
                     }},
        {"type", static_cast<std::uint8_t>(type)}};
    result["changes"].emplace_back(jsonChange);
  });

  return result;
}

const std::unordered_map<pf::mc::LowResPoint, pf::mc::Voxel::Type> &pf::mc::Chunk::getChanges() const {
  return changes;
}

void pf::mc::Chunk::setChanges(const std::unordered_map<LowResPoint, Voxel::Type> &changes) {
  Chunk::changes = changes;
  for (const auto &[position, type] : changes) {
    setVoxel(position, type);
  }
}
