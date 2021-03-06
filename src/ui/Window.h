/**
* @file Window.h
* @brief Window taking care of user events and main loop.
* @author Petr Flajšingr
* @date 25.10.2021
*/

#ifndef OPENGL_TEMPLATE_SRC_UI_WINDOW_H
#define OPENGL_TEMPLATE_SRC_UI_WINDOW_H

#include <GLFW/glfw3.h>
#include <chrono>
#include <concepts>
#include <functional>
#include <optional>
#include <pf_common/enums.h>
#include <queue>
#include <string>

namespace pf::ogl {

enum class MouseButton { Left = 1 << 0,
                         Right = 1 << 1,
                         Middle = 1 << 2,
                         None = 1 << 3 };

enum class MouseEventType {
  Down = 1 << 0,
  Up = 1 << 1
};

enum class KeyEventType { Up = 1 << 0,
                          Pressed = 1 << 1,
                          Repeat = 1 << 2 };
/**
 * @brief Keyboard modifier/state keys.
 */
enum class ModifierKey {
  Shift = 1 << 0,
  Control = 1 << 1,
  Alt = 1 << 2,
  Super = 1 << 3,
  CapsLock = 1 << 4,
  NumLock = 1 << 5
};

/**
 * @brief GLFW window providing callbacks for input and window manipulation.
 *
 * A call to initialize is required to setup GLFW and load OpenGL functions.
 */
class Window {
 public:
  Window(std::size_t width, std::size_t height, std::string title);
  ~Window();

  /**
   * Setup GLFW window and load OpenGL functions using GLAD.
   * @return nullopt if nothing went wrong, otherwise a string containing details of the error
   */
  [[nodiscard]] std::optional<std::string> initialize();
  /**
   * Show window and block the current thread. Starts the main loop.
   */
  void show();
  /**
   * Closes the window and stops main loop in the next tick.
   */
  void close();

  [[nodiscard]] std::size_t getWidth() const;
  [[nodiscard]] std::size_t getHeight() const;
  [[nodiscard]] const std::string &getTitle() const;
  [[nodiscard]] GLFWwindow *getWindowHandle() const;
  /**
   * Called on window resize.
   * @param resizeUserCallback
   */
  void setResizeCallback(std::invocable<std::size_t, std::size_t> auto &&resizeUserCallback) {
    Window::resizeUserCallback = resizeUserCallback;
  }
  /**
   * Called on mouse interaction - both down and up. Provides event type, mouse button and location within the window.
   * @param mouseButtonUserCallback
   */
  void setMouseButtonCallback(std::invocable<MouseEventType, MouseButton, double, double> auto &&mouseButtonUserCallback) {
    Window::mouseButtonUserCallback = mouseButtonUserCallback;
  }
  /**
   * Called for each mouse move. Provides location of the mouse and delta in x and y.
   * @param mouseMoveUserCallback
   */
  void setMouseMoveCallback(std::invocable<double, double, double, double> auto &&mouseMoveUserCallback) {
    Window::mouseMoveUserCallback = mouseMoveUserCallback;
  }
  /**
   * Called on mouse wheel interaction. Parameters are delta in x and y axes.
   * @param mouseWheelUserCallback
   */
  void setMouseWheelCallback(std::invocable<double, double> auto &&mouseWheelUserCallback) {
    Window::mouseWheelUserCallback = mouseWheelUserCallback;
  }
  /**
   * Called on keyboard interaction. Provides info of event type (up/down), keyboard modifiers which were active and key character.
   * @param keyUserCallback
   */
  void setKeyCallback(std::invocable<KeyEventType, Flags<ModifierKey>, char> auto &&keyUserCallback) {
    Window::keyUserCallback = keyUserCallback;
  }
  /**
   * A function called periodically in each tick.
   * @param mainLoop
   */
  void setMainLoop(std::invocable<double> auto &&mainLoop) {
    Window::mainLoop = mainLoop;
  }
  /**
   * A predicate to ignore user input. Since imgui manages the interaction itself it can be useful to use this in order to avoid handling the event twice.
   * @param predicate
   */
  void setInputIgnorePredicate(std::predicate auto &&predicate) {
    inputIgnorePredicate = predicate;
  }
  /**
   * Enqueue task to be run at the end of current loop.
   */
  void enqueueTask(std::invocable auto &&task) {
    tasks.emplace_back(std::forward<decltype(task)>(task));
  }

  void enqueueDelayedTask(std::invocable auto &&task, std::chrono::milliseconds delay) {
    const auto execTime = std::chrono::steady_clock::now() + delay;
    eventQueue.emplace(
        [c = std::forward<decltype(task)>(task)] {
          c();
        },
        execTime);
  }

 private:
  static void windowSizeCallback(GLFWwindow *window, int width, int height);
  std::function<void(std::size_t, std::size_t)> resizeUserCallback = [](auto, auto) {};

  static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
  std::function<void(MouseEventType, MouseButton, double, double)> mouseButtonUserCallback = [](auto, auto, auto, auto) {};

  static void mousePositionCallback(GLFWwindow *window, double xpos, double ypos);
  std::function<void(double, double, double, double)> mouseMoveUserCallback = [](auto, auto, auto, auto) {};

  static void mouseWheelCallback(GLFWwindow *window, double xpos, double ypos);
  std::function<void(double, double)> mouseWheelUserCallback = [](auto, auto) {};

  static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
  std::function<void(KeyEventType, Flags<ModifierKey>, char)> keyUserCallback = [](auto, auto, auto) {};

  std::function<void(double)> mainLoop = [](auto) {};

  std::function<bool()> inputIgnorePredicate = [] { return false; };

  std::size_t width;
  std::size_t height;
  std::string title;

  bool closed = false;

  std::optional<double> mousePosX = std::nullopt;
  std::optional<double> mousePosY = std::nullopt;

  GLFWwindow *windowHandle = nullptr;

  std::vector<std::function<void()>> tasks{};

  struct DelayEvent {
    static inline auto IdGenerator = 0;
    inline DelayEvent(std::function<void()> fnc, const std::chrono::steady_clock::time_point &execTime)
        : fnc(std::move(fnc)), execTime(execTime) {}
    std::function<void()> fnc;
    std::chrono::steady_clock::time_point execTime;
    std::size_t id = ++IdGenerator;
    inline bool operator<(const DelayEvent &rhs) const {
      return execTime < rhs.execTime && id < rhs.id;
    }

    inline void operator()() const { fnc(); }
  };
  std::priority_queue<DelayEvent, std::vector<DelayEvent>, std::less<>> eventQueue;
};

namespace details {
std::optional<MouseButton> glfwButtonToEvents(int button);
std::optional<KeyEventType> glfwKeyEventToEvents(int key_event);

Flags<pf::ogl::ModifierKey> modToFlags(int mods);
}// namespace details

}// namespace pf::ogl

#endif//OPENGL_TEMPLATE_SRC_UI_WINDOW_H
