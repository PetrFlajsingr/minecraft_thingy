#include "renderers/MinecraftThingyRenderer.h"
#include "ui/UI.h"
#include "utils/files.h"
#include <filesystem>
#include <fmt/format.h>
#include <log.h>
#include <magic_enum.hpp>
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
      [&](std::string msg) {
        fmt::print(msg);
        ui.logMemo->addRecord(msg);
      });

  mainWindow.setInputIgnorePredicate([&] { return ui.imguiInterface->isWindowHovered() || ui.imguiInterface->isKeyboardCaptured(); });

  auto camera = std::make_shared<Camera>();
  bool cameraMoveEnabled = false;
  double frameTime = 0.0;// hack
  mainWindow.setMouseButtonCallback([&](MouseEventType type, MouseButton button, double, double) {
    if (button == MouseButton::Right) {
      cameraMoveEnabled = type == MouseEventType::Down;
    }
  });
  mainWindow.setMouseMoveCallback([&](double, double, double deltaX, double deltaY) {
    if (cameraMoveEnabled) {
      camera->ProcessMouseMovement(deltaX, -deltaY);
      ui.setCameraDirectionText(camera->Front);
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
  pf::mc::MinecraftThingyRenderer renderer{resourcesFolder / "shaders", camera};
  if (const auto initResult = renderer.init(); initResult.has_value()) {
    fmt::print(stderr, "Error during initialization: {}\n", initResult.value());
    return -1;
  }

  ui.moveToOriginButton->addClickListener([&] {
    camera->Position = {0.f, 0.f, 0.f};
  });

  ui.lightPosSlider->addValueListener([&](glm::vec3 pos) {
    const auto normDir = glm::normalize(-pos);
    renderer.setLightDir(normDir);
  }, true);


  double lastFrameTime = 0.0;
  mainWindow.setMainLoop([&](double time) {
    frameTime = time - lastFrameTime;
    lastFrameTime = time;
    renderer.render();
    ui.imguiInterface->render();
  });

  fmt::print("Starting main loop\n");
  mainWindow.show();
  fmt::print("Main loop ended\n");

  saveConfig(config, *ui.imguiInterface);
  return 0;
}
