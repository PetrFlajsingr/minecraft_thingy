//
// Created by xflajs00 on 23.10.2021.
//

#include "MinecraftThingyRenderer.h"
#include <geGL/Program.h>
#include <geGL/Shader.h>
#include <glad/glad.h>
#include <log.h>
#include <noise/PerlinNoiseGenerator.h>
#include <utility>
#include <utils/files.h>
#define STB_IMAGE_IMPLEMENTATION
#include <magic_enum.hpp>
#include <stb/stb_image.h>

pf::mc::MinecraftThingyRenderer::MinecraftThingyRenderer(std::filesystem::path shaderDir,
                                                         const std::filesystem::path &textureDir,
                                                         double renderDistance,
                                                         std::size_t chunkLimit,
                                                         std::shared_ptr<Camera> camera,
                                                         std::size_t windowWidth,
                                                         std::size_t windowHeight) : chunkManager(chunkLimit, renderDistance),
                                                                                     shaderDir(std::move(shaderDir)),
                                                                                     camera(std::move(camera)),
                                                                                     windowWidth(windowWidth),
                                                                                     windowHeight(windowHeight) {
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

  boxBuffer = std::make_shared<Buffer>();
  glm::vec3 p{0, 0, 0};
  boxBuffer->alloc(sizeof(glm::vec3), &p, GL_STATIC_DRAW);
  boxVao = std::make_shared<VertexArray>();
  boxVao->addAttrib(boxBuffer, 0, 3, GL_FLOAT, sizeof(glm::vec3));
}

std::optional<std::string> pf::mc::MinecraftThingyRenderer::init() {
  const auto vertexShaderSrc = readFile(shaderDir / "terrain_mesh.vert");
  if (!vertexShaderSrc.has_value()) {
    return "Could not load 'terrain_mesh.vert'";
  }
  const auto fragmentShaderSrc = readFile(shaderDir / "terrain_mesh.frag");
  if (!fragmentShaderSrc.has_value()) {
    return "Could not load 'terrain_mesh.frag'";
  }
  vertexShader = std::make_shared<Shader>(GL_VERTEX_SHADER, vertexShaderSrc.value());
  fragmentShader = std::make_shared<Shader>(GL_FRAGMENT_SHADER, fragmentShaderSrc.value());

  program = std::make_shared<Program>(vertexShader, fragmentShader);

  const auto vertexBoxShaderSrc = readFile(shaderDir / "cube_render.vert");
  if (!vertexBoxShaderSrc.has_value()) {
    return "Could not load 'cube_render.vert'";
  }
  const auto geometryBoxShaderSrc = readFile(shaderDir / "cube_render.geom");
  if (!geometryBoxShaderSrc.has_value()) {
    return "Could not load 'cube_render.geom'";
  }
  const auto fragmentBoxShaderSrc = readFile(shaderDir / "cube_render.frag");
  if (!fragmentBoxShaderSrc.has_value()) {
    return "Could not load 'cube_render.vert'";
  }
  vertexBoxShader = std::make_shared<Shader>(GL_VERTEX_SHADER, vertexBoxShaderSrc.value());
  geometryBoxShader = std::make_shared<Shader>(GL_GEOMETRY_SHADER, geometryBoxShaderSrc.value());
  fragmentBoxShader = std::make_shared<Shader>(GL_FRAGMENT_SHADER, fragmentBoxShaderSrc.value());
  boxProgram = std::make_shared<Program>(vertexBoxShader, geometryBoxShader, fragmentBoxShader);

  return std::nullopt;
}

void pf::mc::MinecraftThingyRenderer::render() {
  chunkManager.generateChunks(camera->Position);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);

  if (wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  program->use();
  const auto projectionMatrix = getProjectionMatrix();
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

  if (drawOutline && outlinePosition.has_value()) {
    glDisable(GL_CULL_FACE); // wrong order in geom shader
    //boxBuffer->setData(&*outlinePosition);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    boxProgram->use();
    boxProgram->setMatrix4fv("mvp", &mvp[0][0]);
    boxProgram->set1f("sideLen", 1.1f);
    const auto pos = glm::vec3{*outlinePosition};
    boxProgram->set3fv("hack", &pos[0]);
    boxVao->bind();
    glDrawArrays(GL_POINTS, 0, 1);
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

glm::ivec3 pf::mc::MinecraftThingyRenderer::getLookedAtCoordinatesFromDepth() const {
  float depth;
  glReadPixels(windowWidth / 2, windowHeight / 2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
  const auto viewport = glm::vec4{0, 0, windowWidth, windowHeight};
  const auto windowCoordinates = glm::vec3{windowWidth / 2, windowHeight / 2, depth};
  const auto worldCoordinates = glm::unProject(windowCoordinates, camera->GetViewMatrix(), getProjectionMatrix(), viewport);
  return glm::ivec3{worldCoordinates};
}

glm::mat4 pf::mc::MinecraftThingyRenderer::getProjectionMatrix() const {
  return glm::perspective(glm::radians(90.0f), 4.0f / 3, 0.1f, 1000.f);
}

void pf::mc::MinecraftThingyRenderer::setDrawOutline(bool drawOutline) {
  MinecraftThingyRenderer::drawOutline = drawOutline;
}

void pf::mc::MinecraftThingyRenderer::userMouseMove() {
  reloadOutlineInfo();
}

void  pf::mc::MinecraftThingyRenderer::userDestroy() {
  const auto castResult = chunkManager.castRay(camera->Position, camera->Front);
  if (castResult.has_value()) {
    chunkManager.setVoxel(castResult->coords, Voxel::Type::Empty);
    reloadOutlineInfo();
  }
}

void  pf::mc::MinecraftThingyRenderer::userBuild(pf::mc::Voxel::Type type) {
  if (outlinePosition.has_value()) {
    chunkManager.setVoxel(*outlinePosition, type);
    reloadOutlineInfo();
  }
}

void pf::mc::MinecraftThingyRenderer::setOutlineType(pf::mc::Outline outlineType) {
  MinecraftThingyRenderer::outlineType = outlineType;
}

std::optional<pf::mc::Voxel> pf::mc::MinecraftThingyRenderer::getActiveVoxel() const {
  if (!outlinePosition.has_value()) {
    return std::nullopt;
  }
  return chunkManager.getVoxel(outlinePosition.value());
}

void pf::mc::MinecraftThingyRenderer::reloadOutlineInfo() {
  if (drawOutline) {
    const auto castResult = chunkManager.castRay(camera->Position, camera->Front);
    if (castResult.has_value()) {
      outlinePosition = castResult->coords;
      if (outlineType == Outline::Neighbor) {
        switch (castResult->face) {
          case Direction::Up: outlinePosition->y += 1; break;
          case Direction::Down: outlinePosition->y -= 1; break;
          case Direction::Left: outlinePosition->x -= 1; break;
          case Direction::Right: outlinePosition->x += 1; break;
          case Direction::Forward: outlinePosition->z -= 1; break;
          case Direction::Backward: outlinePosition->z += 1; break;
        }
      }
    } else {
      outlinePosition = std::nullopt;
    }
  }
}

double pf::mc::MinecraftThingyRenderer::getWorldSeed() const {
  return chunkManager.getSeed();
}

void pf::mc::MinecraftThingyRenderer::setWorldSeed(double seed) {
  chunkManager.resetWithSeed(seed);
}

pf::mc::ChunkManager &pf::mc::MinecraftThingyRenderer::getChunkManager() {
  return chunkManager;
}
