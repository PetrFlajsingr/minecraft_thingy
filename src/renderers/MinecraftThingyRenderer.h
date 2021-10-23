//
// Created by xflajs00 on 23.10.2021.
//

#ifndef OPENGL_TEMPLATE_SRC_RENDERERS_MINECRAFTTHINGYRENDERER_H
#define OPENGL_TEMPLATE_SRC_RENDERERS_MINECRAFTTHINGYRENDERER_H

#include "Renderer.h"
#include <Camera.h>
#include <world/Chunk.h>
#include <filesystem>
#include <memory>
#include <geGL/Texture.h>

namespace pf::mc {

class MinecraftThingyRenderer : public ogl::Renderer {
 public:
  MinecraftThingyRenderer(std::filesystem::path shaderDir, const std::filesystem::path& textureDir, std::shared_ptr<Camera> camera);

  std::optional<std::string> init() override;
  void render() override;

  void setLightDir(const glm::vec3 &lightDir);

  void setWireframe(bool wireframe);

 private:
  Chunk chunk;
  std::filesystem::path shaderDir;
  std::shared_ptr<Camera> camera;
  std::shared_ptr<Texture> blockTextureAtlas;

  glm::vec3 lightDir;

  bool wireframe = false;

  std::shared_ptr<Shader> vertexShader;
  std::shared_ptr<Shader> fragmentShader;
  std::shared_ptr<Program> program;
};

}// namespace pf::mc
#endif//OPENGL_TEMPLATE_SRC_RENDERERS_MINECRAFTTHINGYRENDERER_H
