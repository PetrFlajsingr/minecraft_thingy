//
// Created by xflajs00 on 23.10.2021.
//

#ifndef MINECRAFT_THINGY_SRC_UI_UI_H
#define MINECRAFT_THINGY_SRC_UI_UI_H

#include <world/Voxel.h>
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements.h>
#include <pf_imgui/layouts/layouts.h>
#include <pf_imgui/elements/plots/SimplePlot.h>
#include <GLFW/glfw3.h>
#include <pf_common/enums.h>

ENABLE_PF_ENUM_OUT_FOR_NAMESPACE(pf::mc)

namespace pf::mc {
struct UI {
  UI(const toml::table &config, GLFWwindow *windowHandle);

  void setCameraPositionText(glm::vec3 position);
  void setCameraDirectionText(glm::vec3 dir);

  void log(std::string_view msg, auto ...args) {
    logMemo->addRecord(fmt::format(msg, std::forward<decltype(args)...>(args)...));
  }

  // clang-format off
  ui::ig::AppMenuBar *appMenuBar;
    ui::ig::SubMenu *fileSubMenu;
  ui::ig::Window *cameraWindow;
    ui::ig::BoxLayout *camWindowLayout;
      ui::ig::Text *camPosLabel;
      ui::ig::Text *camDirLabel;
      ui::ig::Button *moveToOriginButton;
      ui::ig::Checkbox *showWireframeCheckbox;
      ui::ig::Checkbox *clipCheckbox;
      ui::ig::Checkbox *frustumCullingCheckbox;
  ui::ig::Window *logWindow;
    ui::ig::Memo *logMemo;
  ui::ig::Window *lightingWindow;
    ui::ig::Slider<glm::vec3> *lightPosSlider;
  ui::ig::Window *infoWindow;
    ui::ig::SimplePlot *fpsCurrentPlot;
    ui::ig::SimplePlot *fpsAveragePlot;
    ui::ig::Text *fpsLabel;
  ui::ig::Window *controlsWindow;
    ui::ig::RadioGroup *destroyAddRadioGroup;
      ui::ig::RadioButton *addVoxelBtn;
      ui::ig::RadioButton *destroyVoxelBtn;
    ui::ig::Combobox<Voxel::Type> *voxelTypeCombobox;
  ui::ig::Window *worldWindow;
    ui::ig::Input<double> *seedInput;
    ui::ig::Button *generateButton;
  // clang-format on

  std::unique_ptr<ui::ig::ImGuiInterface> imguiInterface;
};
}
#endif//MINECRAFT_THINGY_SRC_UI_UI_H
