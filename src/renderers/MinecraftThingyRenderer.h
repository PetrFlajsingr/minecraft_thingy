//
// Created by xflajs00 on 23.10.2021.
//

#ifndef OPENGL_TEMPLATE_SRC_RENDERERS_MINECRAFTTHINGYRENDERER_H
#define OPENGL_TEMPLATE_SRC_RENDERERS_MINECRAFTTHINGYRENDERER_H

#include "Renderer.h"
#include <Camera.h>
#include <filesystem>
#include <geGL/Texture.h>
#include <memory>
#include <world/ChunkManager.h>

namespace pf::mc {

enum class Outline {
  Voxel,
  Neighbor
};

class MinecraftThingyRenderer : public ogl::Renderer {
 public:
  MinecraftThingyRenderer(std::filesystem::path shaderDir,
                          const std::filesystem::path &textureDir,
                          double renderDistance,
                          std::size_t chunkLimit,
                          std::shared_ptr<Camera> camera,
                          std::size_t windowWidth,
                          std::size_t windowHeight);

  std::optional<std::string> init() override;
  void userMouseMove();

  void userDestroy();
  void userBuild(Voxel::Type type);

  void render() override;

  void setLightDir(const glm::vec3 &lightDir);

  void setWireframe(bool wireframe);
  void setShowFrustumCulling(bool showFrustumCulling);

  void setDrawOutline(bool drawOutline);
  void setOutlineType(Outline outlineType);

  [[nodiscard]] std::optional<Voxel> getActiveVoxel() const;

 private:
  glm::ivec3 getLookedAtCoordinatesFromDepth() const;

  void reloadOutlineInfo();

  glm::mat4 getProjectionMatrix() const;

  ChunkManager chunkManager;
  std::filesystem::path shaderDir;
  std::shared_ptr<Camera> camera;
  std::shared_ptr<Texture> blockTextureAtlas;

  glm::vec3 lightDir{};

  bool wireframe = false;
  bool showFrustumCulling = false;

  std::shared_ptr<Shader> vertexShader;
  std::shared_ptr<Shader> fragmentShader;
  std::shared_ptr<Program> program;

  std::shared_ptr<Shader> vertexBoxShader;
  std::shared_ptr<Shader> geometryBoxShader;
  std::shared_ptr<Shader> fragmentBoxShader;
  std::shared_ptr<Program> boxProgram;

  std::shared_ptr<Buffer> boxBuffer;
  std::shared_ptr<VertexArray> boxVao;

  bool drawOutline = false;
  std::optional<glm::ivec3> outlinePosition;

  std::size_t windowWidth;
  std::size_t windowHeight;

  Outline outlineType;
};

}// namespace pf::mc
#endif//OPENGL_TEMPLATE_SRC_RENDERERS_MINECRAFTTHINGYRENDERER_H
