#include <Fission/core/window.hpp>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <Fission/core/input/keys.hpp>
#include "../internal.hpp"
#include <format.hpp>

using namespace fs;

namespace fs {
    // vvv  Needed for accessing private member variables
    struct Window_Proxy {
        static void on_glfw_cursor_position(GLFWwindow* glfw_window, double x, double y);
    };
}

void on_glfw_error(int error, char const* description) {
    log::error(fmt::format("GLFW: {}", description));
}

void on_glfw_frame_buffer_resize(GLFWwindow*, int width, int height) {
    engine.flags |= Engine::Graphics_Recreate_Swap_Chain;
}

void Window_Proxy::on_glfw_cursor_position(GLFWwindow* glfw_window, double x, double y) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window->mouse_position = {(int)x * 2, (int)y * 2}; // ?? wtf is this

    // Global variables to store deltas
    static double lastX = 0, lastY = 0;
    
    if (window->use_mouse_deltas) {
        window->event_queue.append({
            .type = Event_Mouse_Move_Relative,
            .mouse_move_relative = {
                v2s32(x - lastX, y - lastY),
            }
        });
    }

    lastX = x;
    lastY = y;
}

void on_glfw_mouse_button(GLFWwindow* glfw_window, int button, int action, int mods) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    if (action == GLFW_PRESS)
    window->event_queue.append({
        .type = fs::EventType::Event_Key_Down,
        .key_down = {
            .key_id = (u32)button,
        }
    });
    else if (action == GLFW_RELEASE)
    window->event_queue.append({
        .type = fs::EventType::Event_Key_Up,
        .key_down = {
            .key_id = (u32)button,
        }
    });
}

void on_glfw_key(GLFWwindow* glfw_window, int key, int scancode, int action, int mods) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window->event_queue.append({
        .type = u8(action == GLFW_RELEASE? Event_Key_Up : Event_Key_Down),
        .key_down = {
            .key_id = (u32)key,
        }
    });
}

void on_glfw_character(GLFWwindow* glfw_window, unsigned int codepoint) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window->event_queue.append({
        .type = Event_Character_Input,
        .character_input = {
            .codepoint = codepoint,
        }
    });
}


auto Window::create(Window_Create_Info const& info) -> bool
{
    log::verbose(fmt::format("GLFW version {}", glfwGetVersionString()));
    glfwSetErrorCallback(on_glfw_error);
    log::verbose("Creating Window...");
    glfwInitVulkanLoader(&vkGetInstanceProcAddr);
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _glfw_window = glfwCreateWindow(info.width/2, info.height/2, info.title.str().c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(_glfw_window, this);
    glfwSetFramebufferSizeCallback(_glfw_window, on_glfw_frame_buffer_resize);
    glfwSetCursorPosCallback(_glfw_window, Window_Proxy::on_glfw_cursor_position);
    glfwSetMouseButtonCallback(_glfw_window, on_glfw_mouse_button);
    glfwSetKeyCallback(_glfw_window, on_glfw_key);
    glfwSetCharCallback(_glfw_window, on_glfw_character);
    log::verbose(fmt::format(PLATFORM_"window = {}", (void*)_glfw_window));
    return false;
}

void Window::show() {

}

void Window::close() {
    glfwSetWindowShouldClose(_glfw_window, GLFW_TRUE);
}

void Window::toggle_using_mouse_deltas() {
    use_mouse_deltas = !use_mouse_deltas;
    glfwSetInputMode(_glfw_window, GLFW_CURSOR, use_mouse_deltas ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

Window::~Window() {
    log::verbose("Destroying Window...");
    glfwDestroyWindow(_glfw_window);
    log::verbose(PLATFORM_"GLFW Terminate");
    glfwTerminate();
    log::verbose(PLATFORM_"YOU ARE TERMINATED");
    _glfw_window = nullptr;
}
