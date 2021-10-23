//
// Created by xflajs00 on 23.10.2021.
//

#ifndef MINECRAFT_THINGY_SRC_UI_UI_H
#define MINECRAFT_THINGY_SRC_UI_UI_H

#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements.h>
#include <pf_imgui/layouts/layouts.h>
#include <GLFW/glfw3.h>

namespace pf::mc {
struct UI {
  UI(const toml::table &config, GLFWwindow *windowHandle);

  void setCameraPositionText(glm::vec3 position);
  void setCameraDirectionText(glm::vec3 dir);

  void log(std::string_view msg, auto ...args) {
    logMemo->addRecord(fmt::format(msg, std::forward<decltype(args)...>(args)...));
  }

  // clang-format off
  pf::ui::ig::AppMenuBar *appMenuBar;
    pf::ui::ig::SubMenu *fileSubMenu;
  pf::ui::ig::Window *cameraWindow;
    pf::ui::ig::BoxLayout *camWindowLayout;
      pf::ui::ig::Text *camPosLabel;
      pf::ui::ig::Text *camDirLabel;
      pf::ui::ig::Button *moveToOriginButton;
      pf::ui::ig::Checkbox *showWireframeCheckbox;
  pf::ui::ig::Window *logWindow;
    pf::ui::ig::Memo *logMemo;
  pf::ui::ig::Window *lightingWindow;
    pf::ui::ig::Slider<glm::vec3> *lightPosSlider;

  // clang-format on

  std::unique_ptr<pf::ui::ig::ImGuiInterface> imguiInterface;
};
}
#endif//MINECRAFT_THINGY_SRC_UI_UI_H
