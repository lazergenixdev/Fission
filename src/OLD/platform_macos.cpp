#include "Fission/core/engine.hpp"
#include "Fission/core/input/keys.hpp"
#include "GLFW/glfw3.h"
#include <ctime>
#include <time.h>
#include <sys/sysctl.h> // --> sysctlbyname
#include <vulkan/vulkan.h>

#include "platform_unix.cpp"

char _platform_version_string_buffer[64];

fs::string platform_version = []() {
    // https://stackoverflow.com/a/65649178
    // Thanks Apple, very cool
    char    os_temp [20] = "";
    char   *os_temp_ptr  = os_temp;
    size_t  os_temp_len  = sizeof(os_temp);
    size_t  os_temp_left = 0;
    int     rslt         = 0;

    int major = 0, minor = 0, point = 0;
    
    rslt = sysctlbyname ( "kern.osproductversion", os_temp, &os_temp_len, NULL, 0 );
    if (rslt != 0) {
        // just silently fail, who cares
        goto format;
    }
    
    os_temp_left = os_temp_len; /* length of string returned */
    {
        int temp = atoi ( os_temp_ptr );
        major = temp;
    }
    major = atoi ( os_temp_ptr );
    
    while ( os_temp_left > 0 && *os_temp_ptr != '.' )
    {
        os_temp_left--;
        os_temp_ptr++;
    }
    os_temp_left--;
    os_temp_ptr++;
    minor = atoi ( os_temp_ptr );
    
    while ( os_temp_left > 0 && *os_temp_ptr != '.' )
    {
        os_temp_left--;
        os_temp_ptr++;
    }
    os_temp_left--;
    os_temp_ptr++;
    point = atoi ( os_temp_ptr );
    
    format:
    int len = snprintf(_platform_version_string_buffer,
        sizeof(_platform_version_string_buffer),
        "macOS X (%d.%d.%d)",
        major, minor, point);
    
    return fs::string { _platform_version_string_buffer, len };
}();

char _cpu_name_string_buffer[64];

fs::string cpu_name = []() {
    size_t len = sizeof(_cpu_name_string_buffer);
    sysctlbyname ( "machdep.cpu.brand_string", _cpu_name_string_buffer, &len, NULL, 0 );
    return fs::string { _cpu_name_string_buffer, len - 1 };
}();

using namespace fs;

void os::show_error_dialog(string const&, string const&) {
    // not implemented
}

FISSION_NAMESPACE_BEGIN

void Engine::run() {
    log::verbose(PLATFORM_LOG_PREFIX "starting message loop...");

    while (!glfwWindowShouldClose(engine.window._glfw_window))
        glfwWaitEvents();

    // invalidate main window
    //engine.window._handle = NULL;
}

FISSION_NAMESPACE_END

using namespace fs;

void on_glfw_error(int error, char const* description) {
    log::error(fmt::format("GLFW: {}", description));
}

void on_glfw_frame_buffer_resize(GLFWwindow*, int width, int height) {
    engine.flags |= Engine::Graphics_Recreate_Swap_Chain;
}

void on_glfw_cursor_position(GLFWwindow* glfw_window, double x, double y) {
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
    fs::log::debug(fmt::format("codepoint: U+{:x}", codepoint));
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
    glfwSetCursorPosCallback(_glfw_window, on_glfw_cursor_position);
    glfwSetMouseButtonCallback(_glfw_window, on_glfw_mouse_button);
    glfwSetKeyCallback(_glfw_window, on_glfw_key);
    glfwSetCharCallback(_glfw_window, on_glfw_character);
    return false;
}

void Window::show() {

}

void Window::close() {
    glfwSetWindowShouldClose(_glfw_window, GLFW_TRUE);
}

void Window::set_using_mouse_delta(bool use) {
    glfwSetInputMode(_glfw_window, GLFW_CURSOR, use ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    use_mouse_deltas = use;
}

Window::~Window() {
    log::verbose("Destroying Window...");
    glfwDestroyWindow(_glfw_window);
    log::verbose(PLATFORM_LOG_PREFIX "GLFW Terminate");
    glfwTerminate();
    log::verbose(PLATFORM_LOG_PREFIX "YOU ARE TERMINATED");
    _glfw_window = nullptr;
}
