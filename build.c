#include "tools/build.h"
#include "examples/build.h"

void create_build_directories(void)
{
	scoped_log(WARNING) {
		check(mkdir_if_not_exists("bin"));
		check(mkdir_if_not_exists("bin/" PLATFORM));
		check(mkdir_if_not_exists("bin/" PLATFORM "/int"));
		check(mkdir_if_not_exists(CACHE_DIR));
	}
}

void check_cpp_compiler(void)
{
#if defined(PLATFORM_WINDOWS)
	const char* vcvarsall = cache_find("vcvarsall");
	
	if (vcvarsall == NULL) {		
		vcvarsall = find_file_recursive("C:/Program Files/Microsoft Visual Studio", "vcvarsall.bat");
		
		if (vcvarsall == NULL) {
			nob_log(ERROR, "Could not find `vcvarsall.bat`");
			exit(1);
		}
		nob_log(INFO, "Found `vcvarsall.bat` location " PATH("%s"), vcvarsall);
		
		Cmd cmd = {0};
		cmd_append(&cmd, "cmd.exe", "/c", "call", vcvarsall, "x64", ">nul", "&&", "set");
		cmd_run(&cmd, .stdout_path = cache_file_temp("vcvarsall"));
	}
	
	nob_log(INFO, "Setting environment variables for Microsoft Visual Studio ...");
	{
		String_Builder builder = {0};
		check(read_entire_file(cache_file_temp("vcvarsall"), &builder));
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
#if defined(PLATFORM_WINDOWS)
	const char* url = "https://sdk.lunarg.com/sdk/download/latest/windows/vulkan_sdk.exe";
	const char* sdk_location = getenv("VULKAN_SDK");
	if (sdk_location == NULL) {
		nob_log(ERROR, "Vulkan SDK not found!");
		if (ask("Would you like to install the Vulkan SDK?") == NO)
			return 1;
		
		check(mkdir_if_not_exists("temp"));
		scoped(pushd("temp"), popd())
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
#elif defined(PLATFORM_MACOS)
    // TODO
    //run("curl", "-sO", "https://vulkan.lunarg.com/sdk/latest/mac.json");
    //String_View latest_version_json = read_entire_file_to_sv("mac.json");
    //const char* version = get_json_value(latest_version_json.data, "mac");
    //assert(strlen(version) > 0);
    //run("curl", "-O", "https://sdk.lunarg.com/sdk/download/latest/mac/vulkan_sdk.zip");
    //run("unzip", "vulkan_sdk.zip");
    //const char* installer = temp_sprintf("./vulkansdk-macOS-%s.app/Contents/MacOS/vulkansdk-macOS-%s", version, version);
    //run(installer);
    const char* path = temp_sprintf("%s/VulkanSDK/%s", env_home, d->version);
    if (file_exists(path)) {
        nob_log(INFO, "Found Vulkan SDK " PATH("%s"), path);
    }
    else {
        nob_log(ERROR, "Could not find vulkan sdk [TODO]");
        exit(1);
    }
    //delete_file("vulkan_sdk.zip");

    d->include_path = temp_sprintf("%s/macOS/include", path);
    d->library_path = temp_sprintf("%s/macOS/lib", path);
    assert(file_exists(d->include_path));
    assert(file_exists(d->library_path));

    const char* shared1 = temp_sprintf("libvulkan.%.*s.dylib", vulkan_cut_version(d->version, 1), d->version);
    const char* shared3 = temp_sprintf("libvulkan.%.*s.dylib", vulkan_cut_version(d->version, 3), d->version);
    check(copy_file_if_not_exists(temp_sprintf("%s/%s", d->library_path, shared1), temp_sprintf("bin/" PLATFORM "/%s", shared1)));
    check(copy_file_if_not_exists(temp_sprintf("%s/%s", d->library_path, shared3), temp_sprintf("bin/" PLATFORM "/%s", shared3)));
#else
#endif
	return 0;
}

int fetch_freetype(Dependency* d)
{
	const char* lib_path = library_temp("bin/" PLATFORM, "freetype");
	if (!file_exists(lib_path) || !file_exists(d->include_path))
	scoped_dir("temp")
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
		const char* release_dir = temp_sprintf("%s/Release", build_dir);
		check(copy_file(
			library_temp(release_dir, "freetype"),
			temp_sprintf("../%s", lib_path)
		));
		check(copy_directory_recursively(
			temp_sprintf("%s/include", source_dir),
			temp_sprintf("../%s", d->include_path)
		));
    }
	else nob_log(INFO, "Found %s " PATH("%s"), d->display_name, d->version);
	return 0;
}

Dependency dependencies[] = {
#   include "dependencies.h"
};

int check_dependencies(void)
{
	iterate (dependencies) {
        Dependency* d = &dependencies[i];
        if (d->fetch(d)) return 1;
	}
	return 0;
}


/*
const char* env_home = NULL;

#define CppApplication (1<<0)
#define CppLibrary     (1<<0)

struct CppProgram {
    const char*  name;
    const char*  output_path;
    const char* *library_dirs;
    const char* *include_dirs;
    uint32_t     flags;
};

int vulkan_cut_version(const char* version, int n)
{
    int i = 0, k = 0;
    while (version[i] != 0) {
        if (version[i] == '.') k += 1;
        if (k >= n) break;
        i += 1;
    }
    return i;
}

void fetch_glfw(Dependency* d)
{
    if (!file_exists(d->include_path)) {
        run("git", "clone", "-q", "--depth=1", "--branch", d->version, d->url, d->include_path);
    }
    if (file_exists("bin/" PLATFORM "/lib/libglfw3.a")) {
        nob_log(INFO, "Found %s " PATH("%s"), d->display_name ? d->display_name : d->name, d->include_path);
        goto done;
    }
    run("cmake", "--log-level", "ERROR", "-S", d->include_path, "-B", "bin/" PLATFORM "/int/glfw");
    run("cmake", "--build", "bin/" PLATFORM "/int/glfw", "-j");
    check(copy_file("bin/" PLATFORM "/int/glfw/src/libglfw3.a", "bin/" PLATFORM "/lib/libglfw3.a"));
done:
    d->name = "glfw3";
    d->include_path = "3rd-party/glfw/include";
}

void fetch_git(Dependency* d)
{
    if (file_exists(d->include_path)) {
        nob_log(INFO, "Found %s " PATH("%s"), d->display_name ? d->display_name : d->name, d->include_path);
        goto done;
    }
    run("git", "clone", "-q", "--depth=1", d->url, d->include_path);
done:
    if (d->subfolder) {
        d->include_path = temp_sprintf("%s/%s", d->include_path, d->subfolder);
    }
}

void fetch_header_only(Dependency* d)
{
    if (file_exists(d->include_path)) {
        nob_log(INFO, "Found %s " PATH("%s"), d->display_name ? d->display_name : d->name, d->include_path);
        goto done;
    }
    run("curl", "-so", d->include_path, temp_sprintf(d->url, d->version));
done:
    d->include_path = NULL;
}

Dependency dependencies[] = {
#   include "dependencies.h"
};

void fetch_all_dependencies(void)
{
    forn (len(dependencies)) {
        Dependency* d = &dependencies[i];
        d->fetch(d);
    }
}

#define print_string(s) printf("[%p]", s); if (s) printf(" \"%s\"\n", s); else putchar('\n')

int main(int argc, char* argv[])
{
    NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "3rd-party/build.h", "dependencies.h");

    check(load_variable(&env_home, "HOME"));
    check(mkdir_if_not_exists("bin/"));
    check(mkdir_if_not_exists("bin/" PLATFORM));
    check(mkdir_if_not_exists("bin/" PLATFORM "/int/"));
    check(mkdir_if_not_exists("3rd-party/single-header/"));
    fetch_all_dependencies();

    const char* name = "fission";
    const char* int_output = temp_sprintf("bin/" PLATFORM "/int/%s.o", name);

    check(mkdir_if_not_exists("bin/" PLATFORM "/lib/"));
    cmd_append(&cmd, "ar");
    cmd_append(&cmd, "rvs", temp_sprintf("bin/" PLATFORM "/lib/lib%s.a", name));
    cmd_append(&cmd, int_output);
    check(cmd_run_sync_and_reset(&cmd));

    String_Builder builder = {0};
    const char* format = "{.name = \"%s\", .include_path = \"%s\", .library_path = \"%s\"},\n";
    sb_appendf(&builder, format, name, "include", "bin/" PLATFORM "/lib"); // Fission library
    sb_appendf(&builder, "{.include_path = \"%s\"},\n", "3rd-party/single-header"); // Header-only libraries
    sb_appendf(&builder, "{.include_path = \"%s\"},\n", "3rd-party/glm"); // GLM
    sb_appendf(&builder, "{.include_path = \"%s\"},\n", "3rd-party/fmt/include"); // fmt
    iterate (dependencies) {
        Dependency* d = &dependencies[i];
        if (!d->header_only) {
            sb_appendf(&builder, "{.name = \"%s\",", d->name);
            if (d->include_path) sb_appendf(&builder, ".include_path = \"%s\",", d->include_path);
            if (d->library_path) sb_appendf(&builder, ".library_path = \"%s\",", d->library_path);
            sb_appendf(&builder, "},\n");
        }
    }
    check(write_entire_file("bin/" PLATFORM "/dependencies.h", builder.items, builder.count));

    rebuild_directory("examples");

    if (argc > 2) {
        if (strcmp(argv[1], "run") == 0)
        {
            const char* app = temp_sprintf("./bin/" PLATFORM "/%s", argv[2]);
            run(app);
        }
    }
}
*/

Cpp_Program fission = {
	.source = "src/main.cpp",
	.output_name = "fission",
	.flags = COMPILE_STATIC_LIBRARY,
};

int build_all(void)
{
	if (build()) return 1;
	
	Cpp_Program start = {
		.include_dirs = fission.include_dirs,
		.flags = fission.flags & (~COMPILE_STATIC_LIBRARY),
	};
	
	cmd_append(&start.library_dirs, "bin/" PLATFORM); 
	cmd_append(&start.libraries, "fission");
	
    iterate (dependencies) {
        Dependency* d = &dependencies[i];
		cmd_append(&start.libraries, d->name);
		if (d->library_path)
			cmd_append(&start.library_dirs, d->library_path);
    }
	
	scoped_dir("examples")
		build_all_examples(start);
	return 0;
}

int build(void)
{
	create_build_directories();
	check_cpp_compiler();
	check_dependencies();
	
	cmd_append(&fission.include_dirs, "include");
    iterate (dependencies) {
        Dependency* d = &dependencies[i];
		cmd_append(&fission.include_dirs, d->include_path);
    }
	
	return !compile(fission);
}

int main(int argc, char* argv[])
{
    NOB_GO_REBUILD_URSELF_PLUS(argc, argv,
		"tools/build.h", "examples/build.h", "dependencies.h");

	enum {
		Build_Fission = 0,
		Build_All     = 1,
	} action = Build_Fission;

	forn (argc) {
		if (strcmp(argv[i], "all") == 0) action = Build_All;
		if (strcmp(argv[i], "debug") == 0) fission.flags |= COMPILE_DEBUG;
	}
	
	int r = 0;
	uint64_t start_ns = nanos_since_unspecified_epoch();
	switch (action)
	{
		case Build_All: r = build_all(); break;
		default:        r = build(); break;
	}
	uint64_t duration_ns = nanos_since_unspecified_epoch() - start_ns;
	nob_log(INFO, "Build took %f seconds", (double)(duration_ns/1000)/1e6);
	return r;
}