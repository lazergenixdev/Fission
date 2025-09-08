{
    .name = "vulkan",
    .version = "1.4.321.0",
    .fetch = &fetch_vulkan
},
{
    .name = "freetype",
    .display_name = "FreeType",
    .version = "2-13-3",
    .fetch = &fetch_freetype,
    .include_path = "include/freetype",
    .url = "https://gitlab.freedesktop.org/freetype/freetype/-/archive/VER-%s/freetype-VER-%s.zip?ref_type=tags",
},
/*
{
    .name = "glfw",
    .display_name = "GLFW",
    .version = "3.4",
    .fetch = &fetch_glfw,
    .include_path = "3rd-party/glfw",
    .url = "https://github.com/glfw/glfw.git",
},
*/