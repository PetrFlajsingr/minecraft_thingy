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
  camPosLabel->setTooltip("Current camera position");
  camDirLabel = &camWindowLayout->createChild<Text>("cam_dir_label", "Camera direction: 0.0000x0.0000x1.0000");
  camDirLabel->setTooltip("Current camera direction (front)");
  moveToOriginButton = &camWindowLayout->createChild<Button>("move_origin_btn", "Move to origin");
  moveToOriginButton->setTooltip("Move camera to [0, 0, 0]");
  showWireframeCheckbox = &camWindowLayout->createChild<Checkbox>("wireframe_checkbox", "Wireframe", false, Persistent::Yes);
  showWireframeCheckbox->setTooltip("Enable wireframe rendering");
  clipCheckbox = &camWindowLayout->createChild<Checkbox>("clip_checkbox", "Clip - not implemented", true, Persistent::Yes);
  clipCheckbox->setTooltip("NOT IMPLEMENTED");
  frustumCullingCheckbox = &camWindowLayout->createChild<Checkbox>("frustum_culling_checkbox", "Show frustum culling", false, Persistent::Yes);
  frustumCullingCheckbox->setTooltip("Only render chunks which are fully within the view frustum");

  logWindow = &imguiInterface->createWindow("log_window", "Log");
  logMemo = &logWindow->createChild<Memo>("log_memo", "Log", 0, true, true, 50);
  logWindow->setIsDockable(true);

  lightingWindow = &imguiInterface->createWindow("lighting_window", "Lighting");
  lightPosSlider = &lightingWindow->createChild<Slider<glm::vec3>>("light_pos_slider", "Light pos", -1.0, 1.0, glm::vec3{0, 1, 0}, Persistent::Yes);
  lightPosSlider->setTooltip("Light position - directional light from this direction");
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

  controlsWindow = &imguiInterface->createWindow("controls_window", "Controls");
  controlsWindow->setIsDockable(true);
  destroyAddRadioGroup = &controlsWindow->createChild<RadioGroup>("radio_group_add_destroy", "Voxel interaction");
  addVoxelBtn = &destroyAddRadioGroup->addButton("btn_add_vox", "Add");
  addVoxelBtn->setTooltip("Voxel building mode");
  destroyVoxelBtn = &destroyAddRadioGroup->addButton("btn_destroy_vox", "Destroy");
  destroyVoxelBtn->setTooltip("Voxel destruction mode");
  addVoxelBtn->setValue(true);
  voxelTypeCombobox = &controlsWindow->createChild<Combobox<Voxel::Type>>("combobox_vox_type",
                                                                          "Voxel type",
                                                                          "Select voxel type",
                                                                          magic_enum::enum_values<Voxel::Type>(),
                                                                          ComboBoxCount::Items8,
                                                                          Persistent::Yes);
  voxelTypeCombobox->setTooltip("Type of voxel to build");
  voxelTypeCombobox->setSelectedItem(Voxel::Type::Dirt);

  worldWindow = &imguiInterface->createWindow("world_window", "World");
  worldWindow->setIsDockable(true);
  seedInput = &worldWindow->createChild<Input<double>>("seed_input", "Seed");
  seedInput->setTooltip("Current world seed");
  generateButton = &worldWindow->createChild<Button>("generate_button", "Generate");
  generateButton->setTooltip("Generate the world again with provided seed");
  randomizeButton = &worldWindow->createChild<Button>("randomize_button", "Randomize");
  randomizeButton->setTooltip("Generate the world again with a random seed");

  imguiInterface->setStateFromConfig();
}

void UI::setCameraPositionText(glm::vec3 position) {
  camPosLabel->setText("Camera position: {:.3f}x{:.3f}x{:.3f}", position.x, position.y, position.z);
}

void UI::setCameraDirectionText(glm::vec3 dir) {
  camDirLabel->setText("Camera direction: {:.3f}x{:.3f}x{:.3f}", dir.x, dir.y, dir.z);
}
