#pragma once
#include "/Users/mbz/dev/Test_App/build/xcode4/posix.hpp"
#define FS_PLATFORM_NAME "MacOS"

#define OS_CALL

#define FISSION_PLATFORM_VULKAN_EXTENSION_NAMES \
    "VK_EXT_metal_surface"

#define _os_main() \
int main(int argc, char* argv[])

#include <thread>
#include <vector>

struct GLFWwindow;

__FISSION_BEGIN__

namespace platform
{
    struct Instance {
    //    Instance(int argc, char* argv[]):
    //        argv(argv+0, argv+argc)
    //    {}
    //    std::vector<const char*> argv;
    };

    struct Window {
        GLFWwindow* _glfw_window;
    };

    struct Display {

    };
}

__FISSION_END__

