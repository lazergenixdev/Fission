#include "Fission/core.hpp"
#include <unistd.h>

BEGIN_NAMESPACE(os)

auto init() -> fission::Result
{
	return fission::Success;
}

int fatal_error(string error, string message, source_location location)
{
    printf("\x1b[91m%.*s\x1b[0m: %.*s (\x1b[92m%s\x1b[0m in \x1b[93m%s:%i\x1b[0m)\n",
        (int)error.count, (char*)error.data, (int)message.count, (char*)message.data,
        location.function, location.file, location.line);
    _exit(1); // Syscall, don't want to continue any execution after this
}

END_NAMESPACE()

BEGIN_NAMESPACE(fission)

void Engine::run() {
	{
		scoped_set(logger.prefix, OS_NAME);
    	log::verbose("Starting message loop...");
	}

    while (!glfwWindowShouldClose(window._window))
        glfwWaitEvents();

    engine.flags &=~ Engine::Running;
}

void on_glfw_error(int error, const char* description) {
	log::error("GLFW: (", error, ") ", description);
}

void on_glfw_frame_buffer_resize(GLFWwindow*, int width, int height) {
    NOT_USED(width, height);
    engine.flags |= Engine::Graphics_Recreate_Swap_Chain;
}

void on_glfw_cursor_position(GLFWwindow* glfw_window, double x, double y) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window->mouse_position = {(int)x * 2, (int)y * 2}; // ?? wtf is this

    // Global variables to store deltas
    //static double lastX = 0, lastY = 0;
    //if (window->use_mouse_deltas) {
    //    window->event_queue.append({
    //        .type = Event_Mouse_Move_Relative,
    //        .mouse_move_relative = {
    //            v2s32(x - lastX, y - lastY),
    //        }
    //    });
    //}
    //lastX = x;
    //lastY = y;
}
/*
void on_glfw_mouse_button(GLFWwindow* glfw_window, int button, int action, int mods) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    //if (action == GLFW_PRESS)
    //window->event_queue.append({
    //    .type = fs::EventType::Event_Key_Down,
    //    .key_down = {
    //        .key_id = (u32)button,
    //    }
    //});
    //else if (action == GLFW_RELEASE)
    //window->event_queue.append({
    //    .type = fs::EventType::Event_Key_Up,
    //    .key_down = {
    //        .key_id = (u32)button,
    //    }
    //});
}

void on_glfw_key(GLFWwindow* glfw_window, int key, int scancode, int action, int mods) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    //window->event_queue.append({
    //    .type = u8(action == GLFW_RELEASE? Event_Key_Up : Event_Key_Down),
    //    .key_down = {
    //        .key_id = (u32)key,
    //    }
    //});
}

void on_glfw_character(GLFWwindow* glfw_window, unsigned int codepoint) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    //log::debug(fmt::format("codepoint: U+{:x}", codepoint));
    //window->event_queue.append({
    //    .type = Event_Character_Input,
    //    .character_input = {
    //        .codepoint = codepoint,
    //    }
    //});
}
*/
auto Window::create(Create_Info const& info) -> Result
{
	scoped_set(logger.prefix, OS_NAME);
    log::info("Using GLFW version ", glfwGetVersionString());
    glfwSetErrorCallback(on_glfw_error);
    log::info("Creating Window...");
    glfwInitVulkanLoader(&vkGetInstanceProcAddr);
    if (glfwInit() != GLFW_TRUE) return Failed;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto title = scratch_arena.temp_array<char>(info.title.count + 1);
    memcpy(title.data, info.title.data, info.title.count);
    title[info.title.count] = 0;
    _window = glfwCreateWindow(info.width/2, info.height/2, title.data, nullptr, nullptr);
	if (_window == nullptr) return Failed;
    glfwSetWindowUserPointer(_window, this);
    glfwSetFramebufferSizeCallback(_window, on_glfw_frame_buffer_resize);
    glfwSetCursorPosCallback(_window, on_glfw_cursor_position);
    //glfwSetMouseButtonCallback(_window, on_glfw_mouse_button);
    //glfwSetKeyCallback(_window, on_glfw_key);
    //glfwSetCharCallback(_window, on_glfw_character);

	int w, h;
	glfwGetFramebufferSize(_window, &w, &h);
	log::verbose("Window size: ", w, "x", h);

    return Success;
}

END_NAMESPACE()
