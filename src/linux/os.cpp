#include "common.hpp"
#include <engine.hpp>
#include <GLFW/glfw3.h>

namespace os
{
	void initialize() {
		gfx.create();
	}

	void shutdown() {
		gfx.destroy();
	}

	void show_dialog(char const* title, char const* message) {
        // not implemented
	}
}

void create_window()
{
    engine.window = glfwCreateWindow(1600, 900, __TITLE__, nullptr, nullptr);
}

int main(int argc, char* argv[])
{
    glfwInit();
    create_window();

	os_create_mutex(engine.render_lock);
    pthread_create(&engine.main_thread, nullptr, linux_main, nullptr);

    if (!glfwWindowShouldClose(engine.window))
        glfwPollEvents();

    pthread_join(engine.main_thread, nullptr);
	os_destroy_mutex(engine.render_lock);

    glfwTerminate();
}

