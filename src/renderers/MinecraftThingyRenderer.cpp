//
// Created by xflajs00 on 23.10.2021.
//

#include "MinecraftThingyRenderer.h"
#include <geGL/Program.h>
#include <geGL/Shader.h>
#include <glad/glad.h>
#include <noise/PerlinNoiseGenerator.h>
#include <utility>
#include <utils/files.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

pf::mc::MinecraftThingyRenderer::MinecraftThingyRenderer(std::filesystem::path shaderDir,
                                                         const std::filesystem::path &textureDir,
                                                         double renderDistance,
                                                         std::size_t chunkLimit,
                                                         std::shared_ptr<Camera> camera) : chunkManager(chunkLimit, renderDistance),
                                                                                           shaderDir(std::move(shaderDir)),
                                                                                           camera(std::move(camera)) {
  int width;
  int height;
  int channels;
  auto stbImgDeleter = [](stbi_uc *ptr) {
    stbi_image_free(ptr);
  };
  std::unique_ptr<stbi_uc, decltype(stbImgDeleter)> stbImage(stbi_load((textureDir / "blocks.png").string().c_str(), &width, &height, &channels, 0), stbImgDeleter);
  blockTextureAtlas = std::make_shared<Texture>(GL_TEXTURE_2D, GL_RGBA, 0, width, height);
  blockTextureAtlas->texParameteri(GL_TEXTURE_WRAP_S, GL_REPEAT);
  blockTextureAtlas->texParameteri(GL_TEXTURE_WRAP_T, GL_REPEAT);
  blockTextureAtlas->texParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  blockTextureAtlas->texParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  blockTextureAtlas->setData2D(stbImage.get());
}

std::optional<std::string> pf::mc::MinecraftThingyRenderer::init() {

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
  chunkManager.generateChunks(camera->Position);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  if (wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  program->use();
  const auto projectionMatrix = glm::perspective(glm::radians(90.0f), 4.0f / 3, 0.1f, 1000.f);
  const auto viewMatrix = camera->GetViewMatrix();
  const auto mvp = projectionMatrix * viewMatrix;
  program->setMatrix4fv("mvp", &mvp[0][0]);
  program->set3fv("lightDir", &lightDir[0]);
  blockTextureAtlas->bind(0);
  const auto viewFrustum = math::ViewFrustum::FromProjectionView(viewMatrix, projectionMatrix);
  const auto chunksToRender = chunkManager.getChunksToRender(viewFrustum, showFrustumCulling);
  for (const auto &chunk : chunksToRender) {
    program->set3fv("chunkPosition", &glm::vec3{chunk->getPosition()}[0]);
    chunk->render();
  }
}

void pf::mc::MinecraftThingyRenderer::setLightDir(const glm::vec3 &lightDir) {
  MinecraftThingyRenderer::lightDir = lightDir;
}

void pf::mc::MinecraftThingyRenderer::setWireframe(bool wireframe) {
  MinecraftThingyRenderer::wireframe = wireframe;
}

void pf::mc::MinecraftThingyRenderer::setShowFrustumCulling(bool showFrustumCulling) {
  MinecraftThingyRenderer::showFrustumCulling = showFrustumCulling;
}
