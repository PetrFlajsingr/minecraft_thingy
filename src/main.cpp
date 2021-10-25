#include "renderers/MinecraftThingyRenderer.h"
#include "ui/UI.h"
#include "utils/FPSCounter.h"
#include "utils/files.h"
#include <filesystem>
#include <fmt/format.h>
#include <log.h>
#include <magic_enum.hpp>
#include <pf_imgui/serialization.h>
#include <toml++/toml.h>
#include <ui/Window.h>

/**
 * Load toml config located next to the exe - config.toml
 * @return
 */
toml::table loadConfig() {
  const auto configPath = pf::getExeFolder() / "config.toml";
  const auto configPathStr = configPath.string();
  fmt::print("Loading config from: '{}'\n", configPathStr);
  return toml::parse_file(configPathStr);
}

/**
 * Serialize UI, save it to the config and save the config next to the exe into config.toml
 */
void saveConfig(toml::table config, pf::ui::ig::ImGuiInterface &imguiInterface) {
  const auto configPath = pf::getExeFolder() / "config.toml";
  const auto configPathStr = configPath.string();
  fmt::print("Saving config file to: '{}'\n", configPathStr);
  imguiInterface.updateConfig();
  config.insert_or_assign("imgui", imguiInterface.getConfig());
  auto ofstream = std::ofstream(configPathStr);
  ofstream << config;
}

int main(int argc, char *argv[]) {
  using namespace pf;
  using namespace pf::ogl;
  const auto config = loadConfig();
  const auto resourcesFolder = std::filesystem::path{config["files"]["resources_path"].value<std::string>().value()};

  pf::ogl::Window mainWindow{config["window"]["width"].value<std::size_t>().value(),
                             config["window"]["height"].value<std::size_t>().value(),
                             config["window"]["title"].value<std::string>().value()};
  fmt::print("Initializing window and OpenGL\n");
  if (auto windowInitResult = mainWindow.initialize(); windowInitResult.has_value()) {
    fmt::print(stderr, "Error during initialization: {}\n", windowInitResult.value());
    return -1;
  }
  auto ui = mc::UI{*config["imgui"].as_table(), mainWindow.getWindowHandle()};
  setLogger(
      [&](const std::string &msg) {
        fmt::print(msg + "\n");
        ui.logMemo->addRecord(msg);
      });

  mainWindow.setInputIgnorePredicate([&] { return ui.imguiInterface->isWindowHovered() || ui.imguiInterface->isKeyboardCaptured(); });

  auto camera = std::make_shared<Camera>();
  camera->Position = ui::ig::deserializeGlmVec<glm::vec3>(*config["camera"]["position"].as_array());
  camera->MovementSpeed = camera->MovementSpeed * 10;
  bool cameraMoveEnabled = false;
  bool destructionActive = false;
  double frameTime = 0.0;// hack
  const auto renderDistance = config["chunks"]["render_distance"].value<double>().value();
  const auto chunkLimit = config["chunks"]["max_limit"].value<std::size_t>().value();
  mc::MinecraftThingyRenderer renderer{resourcesFolder / "shaders",
                                       resourcesFolder / "textures",
                                       renderDistance,
                                       chunkLimit,
                                       camera,
                                       mainWindow.getWidth(),
                                       mainWindow.getHeight()};
  if (const auto initResult = renderer.init(); initResult.has_value()) {
    fmt::print(stderr, "Error during initialization: {}\n", initResult.value());
    return -1;
  }

  bool destructionInProgress = false;
  mainWindow.setMouseButtonCallback([&](MouseEventType type, MouseButton button, double, double) {
    if (button == MouseButton::Right) {
      cameraMoveEnabled = type == MouseEventType::Down;
    }
    if (button == MouseButton::Left && cameraMoveEnabled) {
      switch (type) {
        case MouseEventType::Down:
          if (destructionActive) {
            if (const auto voxel = renderer.getActiveVoxel(); voxel.has_value()) {
              destructionInProgress = true;
              mainWindow.enqueueDelayedTask(
                  [&destructionInProgress, &renderer] {
                    log("delayed destruct");
                    if (destructionInProgress) {
                      log("destroying");
                      renderer.userDestroy();
                    } else {
                      log("not destroying");
                    }
                  },
                  voxel->getDestructionTime());
            }
          }
          break;
        case MouseEventType::Up:
          if (destructionActive) {
            destructionInProgress = false;
          } else {
            renderer.userBuild(ui.voxelTypeCombobox->getValue());
          }
          break;
      }
    }
  });
  mainWindow.setMouseMoveCallback([&](double, double, double deltaX, double deltaY) {
    if (cameraMoveEnabled) {
      camera->ProcessMouseMovement(deltaX, -deltaY);
      ui.setCameraDirectionText(camera->Front);
      renderer.userMouseMove();
    }
  });
  mainWindow.setKeyCallback([&](KeyEventType type, pf::Flags<ModifierKey> mods, char key) {
    if (cameraMoveEnabled && (type == KeyEventType::Pressed || type == KeyEventType::Repeat)) {
      std::optional<Camera_Movement> dir = std::nullopt;
      switch (key) {
        case 'W': dir = Camera_Movement::FORWARD; break;
        case 'A': dir = Camera_Movement::LEFT; break;
        case 'S': dir = Camera_Movement::BACKWARD; break;
        case 'D': dir = Camera_Movement::RIGHT; break;
        case 'Q': dir = Camera_Movement::UP; break;
        case 'E': dir = Camera_Movement::DOWN; break;
        default: break;
      }
      if (dir.has_value()) {
        if (mods.is(ModifierKey::Shift)) {
          camera->ProcessKeyboard(dir.value(), frameTime * 10);
        } else {
          camera->ProcessKeyboard(dir.value(), frameTime);
        }
        ui.setCameraPositionText(camera->Position);
      }
    }
  });

  ui.moveToOriginButton->addClickListener([&] {
    camera->Position = {0.f, 0.f, 0.f};
  });

  ui.lightPosSlider->addValueListener([&](glm::vec3 pos) {
    const auto normDir = glm::normalize(-pos);
    renderer.setLightDir(normDir);
  },
                                      true);
  ui.showWireframeCheckbox->addValueListener([&](bool enabled) {
    renderer.setWireframe(enabled);
  },
                                             true);
  ui.frustumCullingCheckbox->addValueListener([&](bool enabled) {
    renderer.setShowFrustumCulling(enabled);
  },
                                              true);
  renderer.setDrawOutline(true);
  ui.addVoxelBtn->addValueListener([&](bool enabled) {
    destructionActive = !enabled;
    renderer.setOutlineType(destructionActive ? mc::Outline::Voxel : mc::Outline::Neighbor);
  },
                                   true);

  ui.generateButton->addClickListener([&] {
    renderer.setWorldSeed(ui.seedInput->getValue());
  });

  ui.randomizeButton->addClickListener([&] {
    const auto seed = generateSeed();
    ui.seedInput->setValue(seed);
    renderer.setWorldSeed(seed);
  });

  ui.seedInput->setValue(renderer.getWorldSeed());

  ui.saveFileButton->addClickListener([&] {
    ui.imguiInterface->openFileDialog(
        "Select target", {ui::ig::FileExtensionSettings{{"ch_bin"}, "Chunk data (.ch_bin)", ImVec4{1, 0, 0, 1}}},
        [&](const auto &selected) {
          const auto &dst = selected[0];
          const auto dataToSave = renderer.getChunkManager().serialize();
          createFile(dst, dataToSave);
        },
        [] {}, ui::ig::Size{500, 400});
  });

  ui.loadFileButton->addClickListener([&] {
    ui.imguiInterface->openFileDialog(
        "Select saved world", {ui::ig::FileExtensionSettings{{"ch_bin"}, "Chunk data (.ch_bin)", ImVec4{1, 0, 0, 1}}},
        [&](const auto &selected) {
          const auto &dst = selected[0];
          auto data = readBinFile(dst);
          if (data.has_value()) {
            renderer.getChunkManager().resetAndDeserialize(data.value());
            ui.seedInput->setValue(renderer.getChunkManager().getSeed());
          }
        },
        [] {}, ui::ig::Size{500, 400});
  });

  double lastFrameTime = 0.0;
  FPSCounter fpsCounter{};
  mainWindow.setMainLoop([&](double time) {
    frameTime = time - lastFrameTime;
    lastFrameTime = time;
    renderer.render();
    fpsCounter.onFrame();
    ui.fpsAveragePlot->addValue(fpsCounter.averageFPS());
    ui.fpsCurrentPlot->addValue(fpsCounter.currentFPS());
    ui.fpsLabel->setText("Average FPS: {}", fpsCounter.averageFPS());
    ui.imguiInterface->render();
  });

  fmt::print("Starting main loop\n");
  mainWindow.show();
  fmt::print("Main loop ended\n");

  saveConfig(config, *ui.imguiInterface);
  return 0;
}
