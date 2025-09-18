
static Dependency dependencies[] = {
{
	.targets = OS_LINUX | OS_WINDOWS | OS_MACOS,
    .name = "vulkan",
    .fetch = &fetch_vulkan
},
{
	.targets = OS_ALL,
    .name = "freetype",
    .display_name = "FreeType",
    .version = "2-14-1",
    .fetch = &fetch_freetype,
    .include_path = "include/freetype",
    .url = "https://gitlab.freedesktop.org/freetype/freetype/-/archive/VER-%s/freetype-VER-%s.zip?ref_type=tags",
},
{
	.targets = OS_LINUX | OS_MACOS,
    .name = "glfw",
    .display_name = "GLFW",
    .version = "3.4",
    .fetch = &fetch_glfw,
    .include_path = "include/glfw",
    .url = "https://github.com/glfw/glfw.git",
},
};

// Platform Dependencies

static Android_Dependency android_dependency = {
	.platform    = "android-35",
	.ndk         = "28.2.13676358",
	.build_tools = "35.0.0",
};
