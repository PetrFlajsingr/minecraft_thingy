//
// Created by xflajs00 on 23.10.2021.
//

#ifndef OPENGL_TEMPLATE_SRC_WORLD_CHUNK_H
#define OPENGL_TEMPLATE_SRC_WORLD_CHUNK_H

#include <geGL/Buffer.h>
#include <geGL/VertexArray.h>
#include <array>
#include <noise/NoiseGenerator.h>
#include "Voxel.h"

namespace pf::mc {

constexpr static std::size_t CHUNK_LEN = 64;
constexpr static std::size_t CHUNK_SIZE = CHUNK_LEN * CHUNK_LEN * CHUNK_LEN;

class Chunk {
 public:
  Chunk(glm::vec3 position, const NoiseGenerator &noiseGenerator);

  void setChanged();

  void update();
  void render();

  [[nodiscard]] Voxel getVoxel(std::size_t x, std::size_t y, std::size_t z) const;
  void setVoxel(std::size_t x, std::size_t y, std::size_t z, Voxel::Type type);

  [[nodiscard]] bool isVoxelFilled(std::size_t x, std::size_t y, std::size_t z) const;
  [[nodiscard]] bool isVoxelFilled(std::size_t index) const;

 private:
  void generateVoxelData(const NoiseGenerator &noiseGenerator);

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
  glm::vec3 position;
  bool changed;
};

}

#endif//OPENGL_TEMPLATE_SRC_WORLD_CHUNK_H
