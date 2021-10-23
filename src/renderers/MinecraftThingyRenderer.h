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

namespace pf::mc {

class MinecraftThingyRenderer : public ogl::Renderer {
 public:
  MinecraftThingyRenderer(std::filesystem::path shaderDir, std::shared_ptr<Camera> camera);

  std::optional<std::string> init() override;
  void render() override;

  void setLightDir(const glm::vec3 &lightDir);

 private:
  Chunk chunk;
  std::filesystem::path shaderDir;
  std::shared_ptr<Camera> camera;

  glm::vec3 lightDir;

  std::shared_ptr<Shader> vertexShader;
  std::shared_ptr<Shader> fragmentShader;
  std::shared_ptr<Program> program;
};

}// namespace pf::mc
#endif//OPENGL_TEMPLATE_SRC_RENDERERS_MINECRAFTTHINGYRENDERER_H