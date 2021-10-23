//
// Created by xflajs00 on 23.10.2021.
//

#include "MinecraftThingyRenderer.h"
#include <geGL/Program.h>
#include <geGL/Shader.h>
#include <glad/glad.h>
#include <utils/files.h>

#include <utility>

pf::mc::MinecraftThingyRenderer::MinecraftThingyRenderer(std::filesystem::path shaderDir, std::shared_ptr<Camera> camera) : shaderDir(std::move(shaderDir)), camera(std::move(camera)) {
}

std::optional<std::string> pf::mc::MinecraftThingyRenderer::init() {
  chunk.setVoxel(0, 0, 0, Voxel::Type::Gravel);

  const auto vertexShaderSrc = readFile(shaderDir / "mvp_tex_passthrough.vert");
  if (!vertexShaderSrc.has_value()) {
    return "Could not load 'mvp_tex_passthrough.vert'";
  }
  const auto fragmentShaderSrc = readFile(shaderDir / "mvp_tex_passthrough.frag");
  if (!fragmentShaderSrc.has_value()) {
    return "Could not load 'mvp_tex_passthrough.frag'";
  }
  vertexShader = std::make_shared<Shader>(GL_VERTEX_SHADER, vertexShaderSrc.value());
  fragmentShader = std::make_shared<Shader>(GL_FRAGMENT_SHADER, fragmentShaderSrc.value());

  program = std::make_shared<Program>(vertexShader, fragmentShader);

  return std::nullopt;
}

void pf::mc::MinecraftThingyRenderer::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //glEnable(GL_CULL_FACE);
  //glEnable(GL_DEPTH_TEST);

  program->use();
  const auto projectionMatrix = glm::perspective(glm::radians(90.0f), 4.0f / 3, 0.1f, 1000.f);
  const auto viewMatrix = camera->GetViewMatrix();
  const auto mvp = projectionMatrix * viewMatrix;
  program->setMatrix4fv("mvp", &mvp[0][0]);
  chunk.render();
}