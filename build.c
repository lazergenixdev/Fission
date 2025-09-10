#include "tools/build.h"
#include "examples/build.h"

void create_build_directories(void)
{
	scoped_log(WARNING) {
		check(mkdir_if_not_exists("bin"));
		compiler.output_dir = temp_sprintf("bin/%s", target_name(compiler.target_os));
		check(mkdir_if_not_exists(compiler.output_dir));
		compiler.intermediate_dir = temp_sprintf("%s/int", compiler.output_dir);
		check(mkdir_if_not_exists(compiler.intermediate_dir));
		compiler.cache_dir = temp_sprintf("%s/cache", compiler.output_dir);
		check(mkdir_if_not_exists(compiler.cache_dir));
	}
}

void check_cpp_compiler(void)
{
#if OS == OS_WINDOWS
	const char* vcvarsall_cache = cache_file_temp("vcvarsall");
	String_Builder builder = {0};
	if (!read_entire_file(vcvarsall_cache, &builder))
	{
		nob_log(INFO, "Looking for file `vcvarsall.bat` ...");
		const char* location = find_file_recursive("C:/Program Files/Microsoft Visual Studio", "vcvarsall.bat");
		
		if (location == NULL) {
			nob_log(ERROR, "Could not find `vcvarsall.bat`");
			exit(1);
		}
		nob_log(INFO, "Found `vcvarsall.bat` location " PATH("%s"), location);
		
		run("cmd.exe", "/c", "call", location, "x64", ">nul", "&&", "set", ">", vcvarsall_cache);
		check(read_entire_file(vcvarsall_cache, &builder));
	}
	
	nob_log(INFO, "Setting environment variables for Microsoft Visual Studio ...");
	{
		const char* name = builder.items;
		const char* value = NULL;
		for (int i = 0; i < builder.count; ++i)
		{
			char ch = builder.items[i];
			if (ch == '=') {
				builder.items[i] = '\0';
				value = builder.items + i + 1;
			}
			if (ch == '\r') {
				builder.items[i] = '\0';
				SetEnvironmentVariableA(name, value);
				name = builder.items + i + 1;
			}
			if (ch == '\n') {
				name = builder.items + i + 1;
			}
		}
	}
#endif
}

int fetch_vulkan(Dependency* d)
{
#if OS == OS_WINDOWS
	const char* url = "https://sdk.lunarg.com/sdk/download/latest/windows/vulkan_sdk.exe";
	const char* sdk_location = getenv("VULKAN_SDK");
	if (sdk_location == NULL) {
		nob_log(ERROR, "Vulkan SDK not found!");
		if (ask("Would you like to install the Vulkan SDK?") == NO)
			exit(1);
		
		scoped_temp()
		{
			if (!file_exists("vulkan_sdk.exe"))
				run("curl", "-O", url);

			root_run("vulkan_sdk.exe",
				"--accept-licenses --default-answer --confirm-command install");
			nob_log(INFO, "Please restart your terminal."); // :(
		}
		exit(0);
	}
	
	nob_log(INFO, "Found Vulkan " PATH("%s"), sdk_location);
	
    d->include_path = temp_sprintf("%s/include", sdk_location);
    d->library_path = temp_sprintf("%s/Lib", sdk_location);
	d->name = "vulkan-1"; // want to link against this library
	
    assert(file_exists(d->include_path));
    assert(file_exists(d->library_path));
#elif OS == OS_MACOS
	const char* url = "https://sdk.lunarg.com/sdk/download/latest/mac/vulkan_sdk.zip";
	const char* home = getenv("HOME");
	const char* sdk_location = find_any_in_directory(temp_sprintf("%s/VulkanSDK", home));
    if (sdk_location == NULL)
	{
		nob_log(ERROR, "Vulkan SDK not found!");
		if (ask("Would you like to install the Vulkan SDK?") == NO)
			exit(1);
		
		scoped_temp()
		{
			if (!file_exists("vulkan_sdk.zip"))
				run("curl", "-O", url);

			const char* installer = find_begins_with_in_directory(".", "vulkansdk-macOS");
			if (installer == NULL) {
    			run("unzip", "vulkan_sdk.zip");
				installer = find_begins_with_in_directory(".", "vulkansdk-macOS");
			}

			nob_log(INFO, "installer: %s", installer);

			run("open", temp_sprintf("%s/", installer), "--args",
				"--accept-licenses", "--default-answer", "--confirm-command", "install");
			exit(0); // why installer exit immediately, so annoying
		}
	//	sdk_location = find_any_in_directory(temp_sprintf("%s/VulkanSDK", home));
	//	assert(sdk_location != NULL);
    }
    else nob_log(INFO, "Found Vulkan SDK " PATH("%s"), sdk_location);
	const char* version = sdk_location;
	sdk_location = temp_sprintf("%s/VulkanSDK/%s", home, sdk_location);

    d->include_path = temp_sprintf("%s/macOS/include", sdk_location);
    d->library_path = temp_sprintf("%s/macOS/lib", sdk_location);
	assert(file_exists(d->include_path));
	assert(file_exists(d->library_path));

	scoped_log(WARNING) {
		const char* shared1 = temp_sprintf("libvulkan.%.*s.dylib", truncate_version(version, 1), version);
		const char* shared3 = temp_sprintf("libvulkan.%.*s.dylib", truncate_version(version, 3), version);
		check(copy_file_if_not_exists(temp_sprintf("%s/%s", d->library_path, shared1), temp_sprintf("%s/%s", compiler.output_dir, shared1)));
		check(copy_file_if_not_exists(temp_sprintf("%s/%s", d->library_path, shared3), temp_sprintf("%s/%s", compiler.output_dir, shared3)));
	}
#endif
	return 0;
}

int fetch_freetype(Dependency* d)
{
	const char* lib_path = library_temp(compiler.output_dir, "freetype");
	if (!file_exists(lib_path) || !file_exists(d->include_path))
	{
		nob_log(ERROR, "FreeType not found!");
		if (ask("Would you like to install the FreeType?") == NO)
			return 1;

		scoped_temp()
		{
			const char* zip_name = temp_sprintf("freetype-VER-%s.zip", d->version);
			if (!file_exists(zip_name))
				run("curl", "-sO", temp_sprintf(d->url, d->version, d->version));
			const char* source_dir = temp_sprintf("freetype-VER-%s", d->version);
			if (!file_exists(source_dir))
				run("unzip", "-q", zip_name);
			const char* build_dir = temp_sprintf("%s/build", source_dir);
			run("cmake", "--log-level", "ERROR",
				"-S", source_dir, "-B", build_dir,
				"-DCMAKE_BUILD_TYPE=Release",
				"-D", "FT_DISABLE_ZLIB=TRUE",
				"-D", "FT_DISABLE_BZIP2=TRUE",
				"-D", "FT_DISABLE_PNG=TRUE",
				"-D", "FT_DISABLE_HARFBUZZ=TRUE",
				"-D", "FT_DISABLE_BROTLI=TRUE",
			);
			run("cmake", "--build", build_dir, "-j", "--config", "Release");
#if defined(OS_WINDOWS)
			build_dir = temp_sprintf("%s/Release", build_dir);
#endif
			const char* dst_lib_path = temp_sprintf("../%s", lib_path);
			if (!file_exists(dst_lib_path))
			check(copy_file(
				library_temp(build_dir, "freetype"),
				dst_lib_path
			));

			const char* dst_include_path = temp_sprintf("../%s", d->include_path);
			if (!file_exists(dst_include_path))
			check(copy_directory_recursively(
				temp_sprintf("%s/include", source_dir),
				dst_include_path
			));
		}
    }
	else nob_log(INFO, "Found %s " PATH("%s"), d->display_name, d->version);
	return 0;
}

int fetch_glfw(Dependency* d)
{
	const char* lib_path = library_temp(compiler.output_dir, "glfw");
	if (!file_exists(lib_path) || !file_exists(d->include_path))
	{
		nob_log(ERROR, "GLFW not found!");
		if (ask("Would you like to install the GLFW?") == NO)
			return 1;

		scoped_temp()
		{
			if (!file_exists("glfw"))
				run("git", "clone", "-q", "--depth=1", "--branch", d->version, d->url);

			const char* build_dir = "glfw/build";
			run("cmake", "--log-level", "ERROR", "-S", "glfw", "-B", build_dir,
				"-DCMAKE_BUILD_TYPE=Release",
			);
			run("cmake", "--build", build_dir, "-j", "--config", "Release");

			const char* dst_lib_path = temp_sprintf("../%s", lib_path);
			if (!file_exists(dst_lib_path))
			check(copy_file(
				library_temp(temp_sprintf("%s/src", build_dir), "glfw3"),
				dst_lib_path
			));

			const char* dst_include_path = temp_sprintf("../%s", d->include_path);
			if (!file_exists(dst_include_path))
			check(copy_directory_recursively("glfw/include", dst_include_path));
		}
	}
	else nob_log(INFO, "Found %s " PATH("%s"), d->display_name, d->version);
	return 0;
}

Dependency dependencies[] = {
#   include "dependencies.h"
};

Cpp_Program fission = {
	.source = "src/main.cpp",
	.output_name = "fission",
	.flags = COMPILE_STATIC_LIBRARY,
};

int check_dependencies(void)
{
	cmd_append(&fission.include_dirs, "include");
	iterate (dependencies) {
        Dependency* d = &dependencies[i];
		if (!(d->targets & compiler.target_os)) continue;
        if (d->fetch(d)) return 1;
		cmd_append(&fission.include_dirs, d->include_path);
	}

	const char* header_only_output = temp_sprintf("%s/header_only" OBJ_EXT, compiler.intermediate_dir);
	const char* header_only_source = "src/header_only.cpp";
	if (needs_rebuild(header_only_output, &header_only_source, 1))
	{
		Cpp_Program header_only = {
			.source = header_only_source,
			.output_name = "header_only",
			.flags = COMPILE_OBJECT,
			.include_dirs = fission.include_dirs,
		};
		if (!compile(header_only)) return 1;
	}
	return 0;
}

int build_fission_all(void)
{
	if (build_fission()) return 1;
	
	Cpp_Program start = {
		.include_dirs = fission.include_dirs,
		.flags = fission.flags & (~COMPILE_STATIC_LIBRARY),
	};
	
	cmd_append(&start.library_dirs, compiler.output_dir); 
	cmd_append(&start.libraries, "fission");
	
    iterate (dependencies) {
        Dependency* d = &dependencies[i];
		if (!(d->targets & compiler.target_os)) continue;
		cmd_append(&start.libraries, d->name);
		if (d->library_path)
			cmd_append(&start.library_dirs, d->library_path);
    }
	
	int result = 0;
	scoped_timer("Build All Examples")
	{
		scoped_dir("examples")
			if (build_all_examples(start))
				return_defer(1);
	}
defer:
	return result;
}

int build_fission(void)
{
	create_build_directories();
	check_cpp_compiler();
	if (check_dependencies()) return 1;
	
	cmd_append(&fission.object_files, "header_only");
	
	int r = 0;
	scoped_timer("Build Fission")
	{
		r = compile(fission);
	}
	return !r;
}

int main(int argc, char* argv[])
{
#ifndef DEBUG
    NOB_GO_REBUILD_URSELF_PLUS(argc, argv,
		"tools/build.h", "examples/build.h", "dependencies.h");
#endif
	enum {
		Build_Fission = 0,
		Build_All     = 1,
	} action = Build_Fission;

	forn (argc) {
		if (strcmp(argv[i], "all") == 0) action = Build_All;
		if (strcmp(argv[i], "debug") == 0) fission.flags |= COMPILE_DEBUG;
	}
	
	compiler.target_os = OS;
	int r = 0;
	switch (action)
	{
		case Build_All: r = build_fission_all(); break;
		default:        r = build_fission(); break;
	}
	nob_log(INFO, r? "\x1b[91mCompilation Failed!\x1b[0m" : "\x1b[92mCompilation Succedded!\x1b[0m");
	return r;
}