#include <vulkan/vulkan.h>
#include <cstdio>

using _int0          = VkDebugUtilsMessageSeverityFlagBitsEXT;
using _int1          = VkDebugUtilsMessageTypeFlagsEXT;
using _callback_data = VkDebugUtilsMessengerCallbackDataEXT const*;

void display_fatal_error(const char* title, const char* what) {

}
void display_fatal_graphics_error(VkResult r, char const* what) {
    printf("graphics error: %s [%i]\n", what, r);
}
void display_fatal_graphics_error(char const* what) {
    printf("graphics error: %s\n", what);
}
VKAPI_ATTR VkBool32 VKAPI_CALL on_graphics_vaidation_error(
    _int0 messageSeverity, _int1 messageType,
    _callback_data pCallbackData, void* pUserData) {
        printf("%s\n%s\n\n", pCallbackData->pMessageIdName, pCallbackData->pMessage);
        return VK_TRUE;
}
