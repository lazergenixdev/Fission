#include "unix.hpp"

#define OS_CALL

#define FISSION_PLATFORM_VULKAN_EXTENSION_NAMES \
    "VK_EXT_metal_surface"

#define os_main() \
    int main(int argc, char* argv[])

struct GLFWwindow;

FISSION_NAMESPACE_BEGIN

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

FISSION_NAMESPACE_END
