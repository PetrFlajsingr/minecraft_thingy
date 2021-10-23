//
// Created by xflajs00 on 23.10.2021.
//

#include "UI.h"
#include <pf_imgui/backends/ImGuiGlfwOpenGLInterface.h>
#include <pf_imgui/styles/dark.h>

using namespace pf::mc;

UI::UI(const toml::table &config, GLFWwindow *windowHandle) {
  using namespace pf::ui::ig;
  imguiInterface = std::make_unique<ImGuiGlfwOpenGLInterface>(ImGuiGlfwOpenGLConfig{
      .windowHandle = windowHandle,
      .flags = {},
      .enableMultiViewport = true,
      .config = config,
      .pathToIconFolder = *config["path_icons"].value<std::string>(),
      .enabledIconPacks = IconPack::FontAwesome5Regular,
      .defaultFontSize = 13.f});
  setDarkStyle(*imguiInterface);

  appMenuBar = &imguiInterface->getMenuBar();
  fileSubMenu = &appMenuBar->addSubmenu("file_submenu", ICON_FA_FILE_ALT " File");
  cameraWindow = &imguiInterface->createWindow("camera_window", "Camera");
  cameraWindow->setIsDockable(true);
  camWindowLayout = &cameraWindow->createChild<BoxLayout>("cam_window_layout", LayoutDirection::TopToBottom, Size::Auto(), AllowCollapse::No, Persistent::Yes);
  camPosLabel = &camWindowLayout->createChild<Text>("cam_pos_label", "Camera position: 0.0000x0.0000x0.0000");
  camDirLabel = &camWindowLayout->createChild<Text>("cam_dir_label", "Camera direction: 0.0000x0.0000x1.0000");
  moveToOriginButton = &camWindowLayout->createChild<Button>("move_origin_btn", "Move to origin");
  showWireframeCheckbox = &camWindowLayout->createChild<Checkbox>("wireframe_checkbox", "Wireframe", false, Persistent::Yes);
  clipCheckbox = &camWindowLayout->createChild<Checkbox>("clip_checkbox", "Clip - not implemented", true, Persistent::Yes);

  logWindow = &imguiInterface->createWindow("log_window", "Log");
  logMemo = &logWindow->createChild<Memo>("log_memo", "Log");
  logWindow->setIsDockable(true);

  lightingWindow = &imguiInterface->createWindow("lighting_window", "Lighting");
  lightPosSlider = &lightingWindow->createChild<Slider<glm::vec3>>("light_pos_slider", "Light pos", -1.0, 1.0, glm::vec3{0, 1, 0}, Persistent::Yes);
  lightingWindow->setIsDockable(true);

  infoWindow = &imguiInterface->createWindow("info_window", "Info");
  fpsCurrentPlot = &infoWindow->createChild<SimplePlot>("fps_plot", "Fps current", PlotType::Lines,
                                                        std::vector<float>{}, std::nullopt, 200, 0, FLT_MAX,
                                                        Size{Width::Auto(), 30});
  fpsAveragePlot = &infoWindow->createChild<SimplePlot>("fps_avg_plot", "Fps average", PlotType::Lines,
                                                        std::vector<float>{}, std::nullopt, 200, 0, FLT_MAX,
                                                        Size{Width::Auto(), 30});
  fpsLabel = &infoWindow->createChild<Text>("fps_label", "Average FPS: {}");
  infoWindow->setIsDockable(true);

  imguiInterface->setStateFromConfig();
}

void UI::setCameraPositionText(glm::vec3 position) {
  camPosLabel->setText("Camera position: {:.3f}x{:.3f}x{:.3f}", position.x, position.y, position.z);
}

void UI::setCameraDirectionText(glm::vec3 dir) {
  camDirLabel->setText("Camera direction: {:.3f}x{:.3f}x{:.3f}", dir.x, dir.y, dir.z);
}
