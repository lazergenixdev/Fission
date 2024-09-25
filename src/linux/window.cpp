#include <Fission/core/window.hpp>
#include <GLFW/glfw3.h>
#include <Fission/core/input/Keys.hpp>
#include <internal.hpp>
#include <format.hpp>

using namespace fs;

void on_glfw_error(int error, char const* description) {
    log::error(fmt::format("GLFW: {}", description));
}

void on_glfw_frame_buffer_resize(GLFWwindow*, int width, int height) {
    engine.flags |= Engine::Graphics_Recreate_Swap_Chain;
}

void on_glfw_cursor_position(GLFWwindow* glfw_window, double x, double y) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window->mouse_position = {(int)x, (int)y};
}

void on_glfw_mouse_button(GLFWwindow* glfw_window, int button, int action, int mods) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    if (action == GLFW_PRESS)
    window->event_queue.append({
        .type = fs::EventType::Event_Key_Down,
        .key_down = {
            .key_id = button,
        }
    });
    if (action == GLFW_RELEASE)
    window->event_queue.append({
        .type = fs::EventType::Event_Key_Up,
        .key_down = {
            .key_id = button,
        }
    });
}

auto Window::create(Window_Create_Info const& info) -> bool
{
    glfwSetErrorCallback(on_glfw_error);
    fs::log::verbose("Creating Window...");
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _glfw_window = glfwCreateWindow(100, 100, __TITLE__, nullptr, nullptr);
    glfwSetFramebufferSizeCallback(_glfw_window, on_glfw_frame_buffer_resize);
    glfwSetCursorPosCallback(_glfw_window, on_glfw_cursor_position);
    glfwSetWindowUserPointer(_glfw_window, this);
    glfwSetMouseButtonCallback(_glfw_window, on_glfw_mouse_button);
    log::verbose(fmt::format("window = {}", (void*)_glfw_window));
    return false;
}

void Window::show() {

}

void Window::close() {
    //glfwDestroyWindow(_glfw_window);
}

Window::~Window() {
    log::verbose("Destroying Window...");
    glfwDestroyWindow(_glfw_window);
    log::verbose("(Linux) GLFW Terminate");
    glfwTerminate();
    log::verbose("(Linux) YOU ARE TERMINATED");
    _glfw_window = nullptr;
}

