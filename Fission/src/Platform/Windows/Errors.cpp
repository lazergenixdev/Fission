#include <DXErr/dxerr.hpp>
#include <vulkan/vulkan.h>
#include <format>

#ifdef FISSION_DEBUG
#define USE_DEBUG_OUTPUT_WINDOW 1
#endif

#define MBT_ERROR  (MB_OK | MB_ICONERROR | MB_SYSTEMMODAL)

using _int0          = VkDebugUtilsMessageSeverityFlagBitsEXT;
using _int1          = VkDebugUtilsMessageTypeFlagsEXT;
using _callback_data = VkDebugUtilsMessengerCallbackDataEXT const*;

void display_win32_fatal_error(WCHAR const* what); // Windows Only
void display_fatal_error(const char* title, const char* what);
void display_fatal_graphics_error(VkResult r, char const* what);
void display_fatal_graphics_error(char const* what);
VKAPI_ATTR VkBool32 VKAPI_CALL on_graphics_vaidation_error(_int0 messageSeverity, _int1 messageType, _callback_data pCallbackData, void* pUserData);

void display_win32_fatal_error(WCHAR const* what) {
    DWORD error = GetLastError();
    HRESULT hr = HRESULT_FROM_WIN32(error);

    WCHAR desc[256];
    WCHAR const* name = DXGetErrorStringW(hr);
    DXGetErrorDescriptionW(hr, desc, std::size(desc));

    auto body = std::format(L"{}\n\nError: {}\nDescription: {}", what, name, desc);

#ifdef USE_DEBUG_OUTPUT_WINDOW
    OutputDebugStringW(body.c_str());
    OutputDebugStringA("\n");
#endif
    MessageBoxW(NULL, body.c_str(), L"Fatal Error :(", MBT_ERROR);
}

void display_fatal_error(const char* title, const char* what) {
#ifdef USE_DEBUG_OUTPUT_WINDOW
    OutputDebugStringA(what);
    OutputDebugStringA("\n");
#endif
    MessageBoxA(NULL, what, title, MBT_ERROR);
}

constexpr const char* vulkan_result_to_string(VkResult result) {
    switch (result)
    {
#define return_string(R) case R: return #R;
        return_string(VK_SUCCESS)
        return_string(VK_NOT_READY)
        return_string(VK_TIMEOUT)
        return_string(VK_EVENT_SET)
        return_string(VK_EVENT_RESET)
        return_string(VK_INCOMPLETE)
        return_string(VK_ERROR_OUT_OF_HOST_MEMORY)
        return_string(VK_ERROR_OUT_OF_DEVICE_MEMORY)
        return_string(VK_ERROR_INITIALIZATION_FAILED)
        return_string(VK_ERROR_DEVICE_LOST)
        return_string(VK_ERROR_MEMORY_MAP_FAILED)
        return_string(VK_ERROR_LAYER_NOT_PRESENT)
        return_string(VK_ERROR_EXTENSION_NOT_PRESENT)
        return_string(VK_ERROR_FEATURE_NOT_PRESENT)
        return_string(VK_ERROR_INCOMPATIBLE_DRIVER)
        return_string(VK_ERROR_TOO_MANY_OBJECTS)
        return_string(VK_ERROR_FORMAT_NOT_SUPPORTED)
        return_string(VK_ERROR_FRAGMENTED_POOL)
        return_string(VK_ERROR_UNKNOWN)
        return_string(VK_ERROR_OUT_OF_POOL_MEMORY)
        return_string(VK_ERROR_INVALID_EXTERNAL_HANDLE)
        return_string(VK_ERROR_FRAGMENTATION)
        return_string(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
        return_string(VK_PIPELINE_COMPILE_REQUIRED)
        return_string(VK_ERROR_SURFACE_LOST_KHR)
        return_string(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
        return_string(VK_SUBOPTIMAL_KHR)
        return_string(VK_ERROR_OUT_OF_DATE_KHR)
        return_string(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
        return_string(VK_ERROR_VALIDATION_FAILED_EXT)
        return_string(VK_ERROR_INVALID_SHADER_NV)
        return_string(VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR)
        return_string(VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR)
        return_string(VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR)
        return_string(VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR)
        return_string(VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR)
        return_string(VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR)
        return_string(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
        return_string(VK_ERROR_NOT_PERMITTED_KHR)
        return_string(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
        return_string(VK_THREAD_IDLE_KHR)
        return_string(VK_THREAD_DONE_KHR)
        return_string(VK_OPERATION_DEFERRED_KHR)
        return_string(VK_OPERATION_NOT_DEFERRED_KHR)
        return_string(VK_ERROR_COMPRESSION_EXHAUSTED_EXT)
        return_string(VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT)
#undef return_string
    default:return "[unknown result value]";
    }
}

void display_fatal_graphics_error(VkResult r, char const* what) {
    auto name = vulkan_result_to_string(r);
    auto body = std::format("{}\n\nError: {}", what, name);

#ifdef USE_DEBUG_OUTPUT_WINDOW
    OutputDebugStringA(body.c_str());
#endif
    MessageBoxA(NULL, body.c_str(), "Fatal Graphics Error :(", MBT_ERROR);
}
void display_fatal_graphics_error(char const* what) {
#ifdef USE_DEBUG_OUTPUT_WINDOW
    OutputDebugStringA(what);
    OutputDebugStringA("\n");
#endif
    MessageBoxA(NULL, what, "Fatal Graphics Error :(", MBT_ERROR);
}

const char* severity_string(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
    switch (severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: return "Verbose";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    return "Info";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return "Warning";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   return "Error";
    default:return "Unknown";
    }
}
UINT severity_icon(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
    switch (severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    return MB_ICONINFORMATION;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return MB_ICONWARNING;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   return MB_ICONERROR;
    default:return MB_ICONEXCLAMATION;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL on_graphics_vaidation_error(
    _int0 severity, _int1 type, _callback_data pCallbackData, void* pUserData)
{
#ifdef USE_DEBUG_OUTPUT_WINDOW
 //   if (!(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)) {
        OutputDebugStringA(pCallbackData->pMessage);
        OutputDebugStringA("\n");
 //   }
#endif
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        auto body = std::format("{}\n\n{}", pCallbackData->pMessageIdName, pCallbackData->pMessage);
        MessageBoxA(NULL, body.c_str(), std::format("Graphics Validation [{}]", severity_string(severity)).c_str(), MB_OK | MB_SYSTEMMODAL | severity_icon(severity));
    }
    return VK_FALSE;
}