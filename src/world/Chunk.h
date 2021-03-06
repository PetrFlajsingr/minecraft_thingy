/**
* @file Chunk.h
* @brief A chunk of game's terrain.
* @author Petr Flajšingr
* @date 25.10.2021
*/


#ifndef OPENGL_TEMPLATE_SRC_WORLD_CHUNK_H
#define OPENGL_TEMPLATE_SRC_WORLD_CHUNK_H

#include "Voxel.h"
#include <array>
#include <geGL/Buffer.h>
#include <geGL/VertexArray.h>
#include <noise/NoiseGenerator.h>
#include <pf_common/concepts/Serializable.h>
#include <pf_common/math/BoundingBox.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <utils/LowResPoint.h>

namespace pf::mc {

/** length of chunk edge */
constexpr static std::size_t CHUNK_LEN = 16;
constexpr static std::size_t CHUNK_SIZE = CHUNK_LEN * CHUNK_LEN * CHUNK_LEN;

class Chunk {
 public:
  using ChangeStorage = std::unordered_map<LowResPoint, Voxel::Type>;
  Chunk(glm::ivec3 position, std::shared_ptr<NoiseGenerator> noiseGenerator);
  Chunk(std::shared_ptr<NoiseGenerator> noiseGenerator, const nlohmann::json &data);

  void setChanged();

  void update();
  void createMesh();
  void render();


  [[nodiscard]] const glm::ivec3 &getPosition() const;
  [[nodiscard]] const glm::vec3 &getCenter() const;
  [[nodiscard]] math::BoundingBox<3> getAABB() const;
  [[nodiscard]] std::size_t getVertexCount() const;

  [[nodiscard]] Voxel getVoxel(glm::ivec3 coords) const;
  void setVoxel(glm::ivec3 coords, Voxel::Type type);

  [[nodiscard]] bool isVoxelFilled(std::size_t x, std::size_t y, std::size_t z) const;
  [[nodiscard]] bool isVoxelFilled(std::size_t index) const;

  [[nodiscard]] bool isModified() const;

  [[nodiscard]] nlohmann::json serialize() const;

  [[nodiscard]] const std::unordered_map<LowResPoint, Voxel::Type> &getChanges() const;

  void setChanges(const std::unordered_map<LowResPoint, Voxel::Type> &changes);

 private:
  void generateVoxelData();
  std::shared_ptr<NoiseGenerator> noiseGenerator;
  bool voxelDataGenerated = false;

  [[nodiscard]] static std::size_t index3Dto1D(std::size_t x, std::size_t y, std::size_t z);

  struct GeometryData {
    std::vector<Voxel::Vertex> vertices;
    std::vector<glm::vec3> normals;
  };
  [[nodiscard]] GeometryData generateGeometry() const;

  std::array<Voxel, CHUNK_SIZE> voxels{Voxel::Type::Empty};
  std::shared_ptr<Buffer> vbo;
  std::shared_ptr<Buffer> nbo;
  std::shared_ptr<VertexArray> vao;
  std::size_t vertexCount = 0;
  glm::ivec3 position;
  glm::vec3 center;
  bool changed;
  bool geometryGenerated = false;
  GeometryData mesh;

  bool modified = false;

  std::unordered_map<LowResPoint, Voxel::Type> changes;
};



}// namespace pf::mc


#endif//OPENGL_TEMPLATE_SRC_WORLD_CHUNK_H
