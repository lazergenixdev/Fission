#ifndef BUILD_H
#define BUILD_H
#include "tools/core_build.h"

#define MSVC_OPTIONS "/nologo", "/std:c++20", "/EHsc-", \
					 "/utf-8", "/GR-", "/MT", \
					 "/W4", "/wd4201"
#define CLANG_OPTIONS "-std=c++20", "-fno-exceptions", \
					  "-Wall", "-Wextra", "-Wpedantic", \
					  "-Wno-nested-anon-types", "-Wno-gnu-anonymous-struct", \
					  "-Wno-missing-field-initializers", \
					  "-Wno-missing-designated-field-initializers"

static struct {
	int         target_os;
	const char* output_dir;
	const char* intermediate_dir;
	const char* cache_dir; // need? only used for vcvars
	const char* company;   // (Android only) com.example
	const char* keystore;  // (Android only)
} build;

static struct {
	const char* sdk_path;
	const char* ndk_path;
	const char* toolchain; // compilers location
	const char* tools;     // command-line tools
	const char* platform;
	const char* build_tools;
} android;

static struct {
	const char* compiler;
} slang;

typedef struct Dependency Dependency;
typedef int (*P_fetch_callback)(Dependency*);

struct Dependency {
	uint32_t          targets;
    const char*       name;
    const char*       display_name;
    const char*       version;
    P_fetch_callback  fetch;
    const char*       include_path;
    const char*       library_path;
    const char*       url;
};

typedef struct {
    const char* platform;
    const char* build_tools;
    const char* ndk;
} Android_Dependency;

typedef enum {
	OPT_SPEED  = 0, // speed is the default
	OPT_NONE   = 1,
} Optimization;

typedef enum {
	COMPILE_DEBUG          = (1<<0),
	COMPILE_STATIC_LIBRARY = (1<<1),
	COMPILE_OBJECT         = (1<<2),
} Compile_Flags;

typedef struct {
	const char*    source;
	Cmd            include_dirs;
	Cmd            object_files; // additional object files to compile into library
	Cmd            libraries;
	Cmd            library_dirs;
	const char*    output_name;
	Compile_Flags  flags;
	Optimization   optimization;
} Cpp_Program;

//! TODO: implement
typedef struct {
	Cpp_Program program;
	const char* asset_dir;
	const char* icons_dir;
} Application;

static void check_cpp_compiler(void);
static void create_build_directories(void);
static Result compile(Cpp_Program program);
static Result build_fission(void);
static Result build_fission_all(void);

//! TODO: remove cache api
static const char* cache_file_temp(const char* name)
{
	return temp_sprintf("%s/%s", build.cache_dir, name);
}
static const char* cache_find(const char* name)
{
	String_Builder builder = {0};
	if (read_entire_file(cache_file_temp(name), &builder))
	{
		sb_append_null(&builder);
		return builder.items;
	}
	return NULL;
}
static void cache_set(const char* name, const char* value)
{
	check(write_entire_file(cache_file_temp(name), value, strlen(value)));
}

Result build_freetype_android(Dependency* d, const char* source_dir);

int fetch_vulkan(Dependency* d)
{
	//! TODO: don't exit after install
#if OS == OS_WINDOWS
	const char* url = "https://sdk.lunarg.com/sdk/download/latest/windows/vulkan_sdk.exe";
	const char* sdk_location = getenv("VULKAN_SDK");
	if (sdk_location == NULL) {
		nob_log(WARNING, "Vulkan SDK not found! Installing Vulkan...");
		
		scoped_temp()
		{
			if (!file_exists("vulkan_sdk.exe"))
				run("curl", "-O", url);

			root_run("vulkan_sdk.exe",
				"--accept-licenses --default-answer --confirm-command install");
			nob_log(INFO, "Please restart your terminal.");
		}
		exit(0);
	}
	
	nob_log(INFO, "Found Vulkan " PATH("%s"), sdk_location);
	
    d->include_path = temp_sprintf("%s/include", sdk_location);
    d->library_path = temp_sprintf("%s/Lib", sdk_location);
	d->name = "vulkan-1"; // want to link against this library
	slang.compiler = temp_sprintf("%s/bin/slangc", sdk_location);
#elif OS == OS_MACOS
	const char* url = "https://sdk.lunarg.com/sdk/download/latest/mac/vulkan_sdk.zip";
	const char* home = getenv("HOME");
	const char* sdk_location = find_begins_with_in_directory(temp_sprintf("%s/VulkanSDK", home), "1.");
    if (sdk_location == NULL)
	{
		nob_log(WARNING, "Vulkan SDK not found! Installing Vulkan...");
		
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
			exit(0);
		}
    }
    else nob_log(INFO, "Found Vulkan SDK " PATH("%s"), sdk_location);
	d->version = sdk_location;
	sdk_location = temp_sprintf("%s/VulkanSDK/%s", home, sdk_location);

    d->include_path = temp_sprintf("%s/macOS/include", sdk_location);
    d->library_path = temp_sprintf("%s/macOS/lib", sdk_location);
	slang.compiler = temp_sprintf("%s/macOS/bin/slangc", sdk_location);
#endif

	assert(file_exists(d->include_path));
	assert(file_exists(d->library_path));
	return 0;
}

int fetch_freetype(Dependency* d)
{
	int result = 0;
	const char* lib_path = library_temp(build.output_dir, "freetype");
	if (build.target_os == OS_ANDROID)
	{
		bool lib1 = file_exists(temp_sprintf("%s/lib/arm64-v8a/libfreetype.a", build.output_dir));
		bool lib2 = file_exists(temp_sprintf("%s/lib/armeabi-v7a/libfreetype.a", build.output_dir));

		if (lib1 && lib2)
		{
			nob_log(INFO, "Found %s " PATH("%s"), d->display_name, d->version);
			return 0;
		}
	}
	if (!file_exists(lib_path) || !file_exists(d->include_path))
	{
		nob_log(WARNING, "FreeType not found! Installing FreeType...");

		scoped_temp()
		{
			const char* zip_name = temp_sprintf("freetype-VER-%s.zip", d->version);
			const char* source_dir = temp_sprintf("freetype-VER-%s", d->version);
			if (!file_exists(zip_name))
				run("curl", "-sO", temp_sprintf(d->url, d->version, d->version));
			if (!file_exists(source_dir))
				run("unzip", "-q", zip_name);

			if (build.target_os == OS_ANDROID)
			{
				result = build_freetype_android(d, source_dir);
				continue;
			}
			
			const char* build_dir = temp_sprintf("%s/build", source_dir);
			run_output("freetype.cmake.log", "cmake",
				"-S", source_dir, "-B", build_dir,
			#if OS == OS_WINDOWS
				"-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded",
			#endif
				"-DCMAKE_BUILD_TYPE=Release",
				"-D", "FT_DISABLE_ZLIB=TRUE",
				"-D", "FT_DISABLE_BZIP2=TRUE",
				"-D", "FT_DISABLE_PNG=TRUE",
				"-D", "FT_DISABLE_HARFBUZZ=TRUE",
				"-D", "FT_DISABLE_BROTLI=TRUE",
			);
			run_output("freetype.build.log", "cmake", "--build", build_dir, "-j", "--config", "Release");
#if OS == OS_WINDOWS
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
	return result;
}

int fetch_glfw(Dependency* d)
{
	const char* lib_path = library_temp(build.output_dir, "glfw");
	if (!file_exists(lib_path) || !file_exists(d->include_path))
	{
		nob_log(WARNING, "GLFW not found! Installing GLFW...");

		scoped_temp()
		{
			if (!file_exists("glfw"))
				run_output("glfw.git.log", "git", "clone", "-q", "--depth=1", "--branch", d->version, d->url);

			const char* build_dir = "glfw/build";
			run_output("glfw.cmake.log", "cmake",
				"-S", "glfw", "-B", build_dir,
				"-DCMAKE_BUILD_TYPE=Release",
			);
			run_output("glfw.build.log", "cmake", "--build", build_dir, "-j", "--config", "Release");

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

#include "dependencies.h"

static Cpp_Program fission = {
	.source = "src/main.cpp",
	.output_name = "fission",
	.flags = COMPILE_STATIC_LIBRARY,
};

Result build_freetype_android(Dependency* d, const char* source_dir)
{
	iterate (android_architectures)
	{
		Android_Architecture arch = android_architectures[i];
		const char* libpath = temp_sprintf("../%s/lib/%s/libfreetype.a", build.output_dir, arch.name);
		const char* build_dir = temp_sprintf("%s/build-%s", source_dir, arch.name);

		if (!file_exists(libpath))
		{
			const char* cmake_log = temp_sprintf("freetype.cmake-%s.log", arch.name);
			const char* cmake_toolchain = temp_sprintf("%s/build/cmake/android.toolchain.cmake", android.ndk_path);
			run_output(cmake_log, "cmake",
				"-S", source_dir, "-B", build_dir,
			#if OS == OS_WINDOWS
				"-G", "Ninja",
			#endif
				"--toolchain", cmake_toolchain,
				temp_sprintf("-DANDROID_ABI=%s", arch.name),
				temp_sprintf("-DANDROID_PLATFORM=%s", android_dependency.platform),
				"-DCMAKE_BUILD_TYPE=Release",
				"-DFT_DISABLE_ZLIB=TRUE",
				"-DFT_DISABLE_BZIP2=TRUE",
				"-DFT_DISABLE_PNG=TRUE",
				"-DFT_DISABLE_HARFBUZZ=TRUE",
				"-DFT_DISABLE_BROTLI=TRUE"
			);

			const char* build_log = temp_sprintf("freetype.build-%s.log", arch.name);
			run_output(build_log, "cmake", "--build", build_dir, "-j", "--config", "Release");
			check(copy_file(temp_sprintf("%s/libfreetype.a", build_dir), libpath));
		}
	}

	const char* dst_include_path = temp_sprintf("../%s", d->include_path);
	if (!file_exists(dst_include_path))
	check(copy_directory_recursively(temp_sprintf("%s/include", source_dir), dst_include_path));
	return Success;
}

static void create_build_directories(void)
{
	scoped_log(WARNING) {
		check(mkdir_if_not_exists("bin"));
		build.output_dir = temp_sprintf("bin/%s", os_name(build.target_os));
		check(mkdir_if_not_exists(build.output_dir));
		build.intermediate_dir = temp_sprintf("%s/int", build.output_dir);
		check(mkdir_if_not_exists(build.intermediate_dir));
		build.cache_dir = temp_sprintf("%s/cache", build.output_dir);
		check(mkdir_if_not_exists(build.cache_dir));

		if (build.target_os == OS_ANDROID)
		{
			check(mkdir_if_not_exists(temp_sprintf("%s/lib", build.output_dir)));
			iterate (android_architectures)
			{
				Android_Architecture arch = android_architectures[i];
				check(mkdir_if_not_exists(temp_sprintf("%s/lib/%s", build.output_dir, arch.name)));
			}
		}
	}
}

static void check_cpp_compiler_android(void)
{
	// Determine SDK location (look at default locations)
	const char* sdk_location = NULL;
#if OS == OS_WINDOWS
	const char* zip = "commandlinetools-win-13114758_latest.zip";
	const char* app_data = getenv("LOCALAPPDATA");
	const char* path1 = temp_sprintf("%s/Android/sdk", app_data);
	const char* path2 = "C:/Program Files/Android/android-sdk";
	if (file_exists(path1)) sdk_location = path1;
	if (!sdk_location && file_exists(path2)) sdk_location = path2;
#elif OS == OS_MACOS
	const char* zip = "commandlinetools-mac-13114758_latest.zip";
	const char* path1 = temp_sprintf("%s/Library/Android/sdk", home_directory());
	if (file_exists(path1)) sdk_location = path1;
#elif OS == OS_LINUX
	const char* zip = "commandlinetools-linux-13114758_latest.zip";
	const char* path1 = temp_sprintf("%s/Android/Sdk", home_directory());
	if (file_exists(path1)) sdk_location = path1;
#endif
	if (!sdk_location) {
		// Install Android SDK
		nob_log(INFO, "Android SDK not found! Installing Android SDK...");
		sdk_location = path1; // use default location

		scoped_temp()
		{
			if (!file_exists(zip))
				run("curl", "-sO", temp_sprintf("https://dl.google.com/android/repository/%s", zip));
			
			mkdir_recursive(path1);
			run("unzip", "-d", path1, zip);
		}
	}
	else nob_log(INFO, "Found Android SDK " PATH("%s"), sdk_location);
	android.sdk_path = sdk_location;

	// Check/Install required tools (https://apilevels.com/)
	const char* sdkmanager = find_file_recursive(sdk_location, OS!=OS_WINDOWS? "sdkmanager" : "sdkmanager.bat");
	const char* sdkroot = temp_sprintf("--sdk_root=%s", sdk_location);
	scoped_dir(sdk_location)
	{
		//! TODO: no hard code
		if (!file_exists("build-tools/35.0.0"))
			run(sdkmanager, sdkroot, "--install", "build-tools;35.0.0");
		if (!file_exists("platforms/android-35"))
			run(sdkmanager, sdkroot, "--install", "platforms;android-35");
		if (!file_exists("ndk/28.2.13676358"))
			run(sdkmanager, sdkroot, "--install", "ndk;28.2.13676358");
		if (!file_exists("platform-tools"))
			run(sdkmanager, sdkroot, "--install", "platform-tools");
	}
	android.tools = path_parent(sdkmanager);
	android.ndk_path = temp_sprintf("%s/ndk/28.2.13676358", sdk_location);
	android.platform = temp_sprintf("%s/platforms/android-35", sdk_location);
	android.build_tools = temp_sprintf("%s/build-tools/35.0.0", sdk_location);
	const char* toolchain_path = temp_sprintf("%s/ndk/28.2.13676358/toolchains/llvm", sdk_location);
	const char* clang = find_file_recursive(toolchain_path, "armv7a-linux-androideabi35-clang++");
	const char* bin = path_parent(clang);
	nob_log(INFO, "Found Android toolchain " PATH("%s"), bin);
	android.toolchain = bin;

	// Check if there is a keystore available
	//! TODO: how to properly handle keystores?
	build.keystore = temp_sprintf("%s/.keystore", home_directory());
	if (!file_exists(build.keystore)) {
		nob_log(WARNING, "Could not locate keystore at " PATH("%s"), build.keystore);
		cmd_append(&cmd, "keytool", "-genkeypair", "-v", "-keystore", build.keystore);
		cmd_append(&cmd, "-alias", "debug", "-storepass", "android", "-keypass", "android", "-keyalg", "RSA", "-keysize", "2048", "-validity", "10000");
		check(cmd_run_sync_and_reset(&cmd));
	}

	const char* setup_env = temp_sprintf("%s/setup-environment" SCRIPT_EXT, build.output_dir);
	if (!file_exists(setup_env))
	{
		String_Builder builder = {0};
	#if OS == OS_WINDOWS
		sb_appendf(&builder, "$Env:PATH += ';%s/platform-tools'\n", android.sdk_path);
	#else
		sb_appendf(&builder, "export PATH=\"%s/platform-tools:$PATH\"\n", android.sdk_path);
	#endif
		sb_appendf(&builder,
			"echo \"Start debugging by using adb\"\n"
			"echo \"'adb install test.apk'\"\n"
			"echo \"'adb shell am start -n dev.lazergenix.test/.MainActivity'\"\n"
			"echo \"'adb logcat FissionEngine:D *:S'\"\n"
		);
		write_entire_file(setup_env, builder.items, builder.count);
	}
}

static void check_cpp_compiler(void)
{
	if (build.target_os == OS_ANDROID)
		return check_cpp_compiler_android();
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

static Result compile_android(Cpp_Program program)
{
	Result result = Success;
	const char* root = get_current_dir_temp();

	Cmd options = {0};
	cmd_append(&options, CLANG_OPTIONS, "-fPIC"); // need PIC for objects?
	cmd_append(&options, temp_sprintf("%s/%s", root, program.source));
	forn (program.include_dirs.count)
		cmd_append(&options, temp_sprintf("-I%s/%s", root, program.include_dirs.items[i]));
	if (program.flags & COMPILE_DEBUG)
		cmd_append(&options, "-g");

	if (program.flags & (COMPILE_OBJECT|COMPILE_STATIC_LIBRARY))
	{
		cmd_append(&options, "-c");
		if (!mkdir_if_not_exists(temp_sprintf("%s/%s.o", build.intermediate_dir, program.output_name)))
			return_defer(Failed);
	}
	else
	{
		cmd_append(&options, "-static-libstdc++", "-shared");
	}
	
	pushd(android.toolchain);

	const char* package = temp_sprintf("%s.%s", build.company, program.output_name);
	const char* namespace = string_replace(package, '.', '_');

	iterate (android_architectures)
	{
		Android_Architecture arch = android_architectures[i];
		cmd_append(&cmd, temp_sprintf(ANDROID_COMPILER, arch.compiler));
		cmd_extend(&cmd, &options);
		if (program.flags & (COMPILE_OBJECT|COMPILE_STATIC_LIBRARY))
			cmd_append(&cmd, "-o", temp_sprintf("%s/%s/%s.o/%s.o", root, build.intermediate_dir, program.output_name, arch.name));
		else {
			cmd_append(&cmd, temp_sprintf("%s/src/platform_android.cpp", root));
			cmd_append(&cmd, temp_sprintf("-D__ANDROID_NAMESPACE__=%s", namespace));
			cmd_append(&cmd, temp_sprintf("-L%s/%s/lib/%s", root, build.output_dir, arch.name));
			forn (program.libraries.count)
				cmd_append(&cmd, temp_sprintf("-l%s", program.libraries.items[i]));
			cmd_append(&cmd, "-landroid", "-llog", "-lvulkan");
			cmd_append(&cmd, "-o", temp_sprintf("%s/%s/lib/%s/lib%s.so", root, build.output_dir, arch.name, program.output_name));
		}
		if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);
	}

	if (program.flags & COMPILE_STATIC_LIBRARY)
	iterate (android_architectures)
	{
		Android_Architecture arch = android_architectures[i];
		cmd_append(&cmd, "./llvm-ar", "rvs");
		cmd_append(&cmd, temp_sprintf("%s/%s/lib/%s/lib%s.a", root, build.output_dir, arch.name, program.output_name));
		cmd_append(&cmd, temp_sprintf("%s/%s/%s.o/%s.o", root, build.intermediate_dir, program.output_name, arch.name));
		forn (program.object_files.count) {
			const char* object_file = temp_sprintf("%s/%s/%s.o/%s.o", root, build.intermediate_dir, program.object_files.items[i], arch.name);
			const char* object_file2 = temp_sprintf("%s/%s/%s.o/%s.o", root, build.intermediate_dir, program.object_files.items[i], arch.name);
			if (file_exists(object_file))
				cmd_append(&cmd, object_file);
			else
				cmd_append(&cmd, object_file2);
		}
		if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);
	}

	if (program.flags & (COMPILE_OBJECT|COMPILE_STATIC_LIBRARY))
		return_defer(Success);

	const char* package_path = string_replace(package, '.', '/');
	
	popd();
	pushd(android.build_tools);

	// Generate R.java
	cmd_append(&cmd, "./aapt", "package", "-f", "-m");
	cmd_append(&cmd, "-S", temp_sprintf("%s/src/android/res", root), "-J", temp_sprintf("%s/%s", root, build.intermediate_dir));
	cmd_append(&cmd, "-M", temp_sprintf("%s/src/android/AndroidManifest.xml", root));
	cmd_append(&cmd, "-I", temp_sprintf("%s/android.jar", android.platform));
	cmd_append(&cmd, "--custom-package", package);
	if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);

	// Generate MainActivity with correct package declaration
	const char* activity_path = temp_sprintf("%s/%s/%s/MainActivity.java", root, build.intermediate_dir, package_path);
	{
		String_Builder builder = {0};
		sb_appendf(&builder, "package %s.%s;\n\n", build.company, program.output_name);
		if (!read_entire_file(temp_sprintf("%s/src/android/dev/lazergenix/fission/MainActivity.java", root), &builder))
			return_defer(Failed);
		if (!write_entire_file(activity_path, builder.items, builder.count))
			return_defer(Failed);
	}

	// Compile .java -> .class
	cmd_append(&cmd, "javac", "-classpath", temp_sprintf("%s/android.jar", android.platform));
	cmd_append(&cmd, "-d", temp_sprintf("%s/%s", root, build.intermediate_dir));
	cmd_append(&cmd, activity_path);
	cmd_append(&cmd, temp_sprintf("%s/%s/%s/R.java", root, build.intermediate_dir, package_path));
	if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);

	// Generate classes.dex
	cmd_append(&cmd, ANDROID_D8, "--min-api", "21");
	cmd_append(&cmd, "--classpath", temp_sprintf("%s/android.jar", android.platform));
	cmd_append_all_ends_with(&cmd, temp_sprintf("%s/%s/%s", root, build.intermediate_dir, package_path), ".class");
	cmd_append(&cmd, "--output", temp_sprintf("%s/%s", root, build.intermediate_dir));
	if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);

	const char* output_apk = temp_sprintf("%s/%s/%s.output.apk", root, build.intermediate_dir, program.output_name);

	// Generate APK
	cmd_append(&cmd, "./aapt", "package", "-f");
	cmd_append(&cmd, "-M", temp_sprintf("%s/src/android/AndroidManifest.xml", root));
	cmd_append(&cmd, "-S", temp_sprintf("%s/src/android/res", root));
	cmd_append(&cmd, "-I", temp_sprintf("%s/android.jar", android.platform));
	cmd_append(&cmd, "-F", output_apk);
	cmd_append(&cmd, "--custom-package", package);
	if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);

	// Add classes.dex
	cmd_append(&cmd, "zip", "-qj", output_apk);
	cmd_append(&cmd, temp_sprintf("%s/%s/classes.dex", root, build.intermediate_dir));
	if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);

	// Add shared libraries
	scoped_dir(temp_sprintf("%s/%s", root, build.output_dir))
	{
		cmd_append(&cmd, "zip", "-qr", temp_sprintf("int/%s.output.apk", program.output_name));
		iterate (android_architectures)
		{
			Android_Architecture arch = android_architectures[i];
			cmd_append(&cmd, temp_sprintf("lib/%s/lib%s.so", arch.name, program.output_name));
		}
		if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);
	}

	// Align APK
	const char* unsigned_apk = temp_sprintf("%s/%s/%s.unsigned.apk", root, build.intermediate_dir, program.output_name);
	cmd_append(&cmd, "./zipalign", "-f", "4", output_apk, unsigned_apk);
	if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);

	// Sign APK
	cmd_append(&cmd, ANDROID_APKSIGNER, "sign", "--ks", build.keystore);
	cmd_append(&cmd, "--ks-key-alias", "debug");
	cmd_append(&cmd, "--ks-pass", "pass:android");
	cmd_append(&cmd, "--v1-signing-enabled", "true", "--v2-signing-enabled", "true");
	cmd_append(&cmd, "--out", temp_sprintf("%s/%s/%s.apk", root, build.output_dir, program.output_name));
	cmd_append(&cmd, unsigned_apk);
	if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);

defer:
	popd();
	return result;
}

static Result compile_windows(Cpp_Program program)
{
	cmd_append(&cmd, "cl.exe", MSVC_OPTIONS);
	if (!(program.flags & COMPILE_DEBUG)) // Debug symbols are terrible with Optimizations
	switch (program.optimization) {
		default: cmd_append(&cmd, "/O2"); break;
		case OPT_NONE: cmd_append(&cmd, "/Od"); break;
	}
	else cmd_append(&cmd, "/Od");
	if ((program.flags & COMPILE_STATIC_LIBRARY) || (program.flags & COMPILE_OBJECT))
		cmd_append(&cmd, "/c");
    cmd_append(&cmd, program.source);
	forn (program.include_dirs.count)
		cmd_append(&cmd, temp_sprintf("/I%s", program.include_dirs.items[i]));
	cmd_append(&cmd, temp_sprintf("/Fo%s/%s.obj", build.intermediate_dir, program.output_name));
	if (program.flags & COMPILE_DEBUG)
	{
		cmd_append(&cmd, "/Zi");
		cmd_append(&cmd, temp_sprintf("/Fd%s/%s.pdb", build.output_dir, program.output_name));
	}
	if (!(program.flags & COMPILE_STATIC_LIBRARY) && !(program.flags & COMPILE_OBJECT))
	{	
		cmd_append(&cmd, temp_sprintf("/Fe%s/%s.exe", build.output_dir, program.output_name));
		cmd_append(&cmd, "/link", "/SUBSYSTEM:WINDOWS");
		forn (program.library_dirs.count)
			cmd_append(&cmd, temp_sprintf("/LIBPATH:\"%s\"", program.library_dirs.items[i]));
		forn (program.libraries.count)
			cmd_append(&cmd, temp_sprintf("%s.lib", program.libraries.items[i]));
	}
    if (!cmd_run_sync_and_reset(&cmd)) return Failed;
	if (program.flags & COMPILE_STATIC_LIBRARY)
	{
		cmd_append(&cmd, "lib", "/nologo");
		cmd_append(&cmd, temp_sprintf("/OUT:%s/%s.lib", build.output_dir, program.output_name));
		cmd_append(&cmd, temp_sprintf("%s/%s.obj", build.intermediate_dir, program.output_name));
		forn (program.object_files.count)
			cmd_append(&cmd, temp_sprintf("%s/%s.obj", build.intermediate_dir, program.object_files.items[i]));
		if (!cmd_run_sync_and_reset(&cmd)) return Failed;
	}
	return Success;
}

static Result compile_macos(Cpp_Program program)
{
	const char* output = NULL;

	// App
	if (!(program.flags & (COMPILE_OBJECT|COMPILE_STATIC_LIBRARY)))
	scoped_log(WARNING)
	{
		String_Builder builder = {0};
		if (!mkdir_if_not_exists(temp_sprintf("%s/%s.app", build.output_dir, program.output_name)))
			return Failed;

		const char* contents_dir = temp_sprintf("%s/%s.app/Contents", build.output_dir, program.output_name);

		if (!mkdir_if_not_exists(contents_dir)) return Failed;
		if (!mkdir_if_not_exists(temp_sprintf("%s/MacOS", contents_dir))) return Failed;
		if (!mkdir_if_not_exists(temp_sprintf("%s/Resources", contents_dir))) return Failed;
		if (!mkdir_if_not_exists(temp_sprintf("%s/Frameworks", contents_dir))) return Failed;

		// Vulkan Loader
		if (!mkdir_if_not_exists(temp_sprintf("%s/Resources/vulkan", contents_dir))) return Failed;
		if (!mkdir_if_not_exists(temp_sprintf("%s/Resources/vulkan/icd.d", contents_dir))) return Failed;
		assert(strcmp(dependencies[0].name, "vulkan") == 0);
		Dependency* vulkan = &dependencies[0];
		const char* shared1 = temp_sprintf("libvulkan.%.*s.dylib", truncate_version(vulkan->version, 1), vulkan->version);
		const char* shared3 = temp_sprintf("libvulkan.%.*s.dylib", truncate_version(vulkan->version, 3), vulkan->version);
		check(copy_file_if_not_exists(temp_sprintf("%s/%s", vulkan->library_path, shared1), temp_sprintf("%s/Frameworks/%s", contents_dir, shared1)));
		check(copy_file_if_not_exists(temp_sprintf("%s/%s", vulkan->library_path, shared3), temp_sprintf("%s/Frameworks/%s", contents_dir, shared3)));
		check(copy_file_if_not_exists(temp_sprintf("%s/libMoltenVK.dylib", vulkan->library_path), temp_sprintf("%s/Frameworks/libMoltenVK.dylib", contents_dir)));
		if (!write_entire_file(temp_sprintf("%s/Resources/vulkan/icd.d/MoltenVK_icd.json", contents_dir), VULKAN_ICD_MACOS, sizeof(VULKAN_ICD_MACOS)-1))
			return Failed;

		output = temp_sprintf("%s/MacOS/%s", contents_dir, program.output_name);

		const char* plist_contents = INFO_PLIST_HEADER
		"<dict>\n"
		"    <key>CFBundleExecutable</key>\n"
		"    <string>%s</string>\n"
		"    <key>CFBundleIdentifier</key>\n"
		"    <string>com.example.myapp</string>\n"
		"    <key>CFBundleName</key>\n"
		"    <string>%s</string>\n"
		"    <key>CFBundleVersion</key>\n"
		"    <string>1.0</string>\n"
		"</dict>\n" INFO_PLIST_FOOTER;

		//! TODO: customizable app name
		sb_appendf(&builder, plist_contents, program.output_name, program.output_name);
		if (!write_entire_file(temp_sprintf("%s/Info.plist", contents_dir), builder.items, builder.count))
			return Failed;
	}

	cmd_append(&cmd, "clang++", CLANG_OPTIONS);
	switch (program.optimization) {
		default: cmd_append(&cmd, "-O2"); break;
		case OPT_NONE: break;
	}
	if ((program.flags & COMPILE_STATIC_LIBRARY) || (program.flags & COMPILE_OBJECT))
		cmd_append(&cmd, "-c");
	cmd_append(&cmd, program.source);
	forn (program.include_dirs.count)
		cmd_append(&cmd, temp_sprintf("-I%s", program.include_dirs.items[i]));

	cmd_append(&cmd, "-o", temp_sprintf("%s/%s.o", build.intermediate_dir, program.output_name));

	if (program.flags & COMPILE_DEBUG)
		cmd_append(&cmd, "-g");

	if (!(program.flags & COMPILE_STATIC_LIBRARY) && !(program.flags & COMPILE_OBJECT))
	{	
		cmd_append(&cmd, "-o", output);
		forn (program.library_dirs.count)
			cmd_append(&cmd, temp_sprintf("-L%s", program.library_dirs.items[i]));
		forn (program.libraries.count)
			cmd_append(&cmd, temp_sprintf("-l%s", program.libraries.items[i]));
		// System Libraries
		cmd_append(&cmd, "-lpthread");
		cmd_append(&cmd, "-rpath", "@executable_path/../Frameworks");
		cmd_append(&cmd, "-framework", "Cocoa", "-framework", "CoreFoundation",
						 "-framework", "CoreAudio", "-framework", "AudioToolbox",
						 "-framework", "IOKit", "-framework", "CoreVideo");
	}
    if (!cmd_run_sync_and_reset(&cmd)) return Failed;
	if (program.flags & COMPILE_STATIC_LIBRARY)
	{
		cmd_append(&cmd, "ar", "rvs", "-c");
		cmd_append(&cmd, temp_sprintf("%s/lib%s.a", build.output_dir, program.output_name));
		cmd_append(&cmd, temp_sprintf("%s/%s.o", build.intermediate_dir, program.output_name));
		forn (program.object_files.count)
			cmd_append(&cmd, temp_sprintf("%s/%s.o", build.intermediate_dir, program.object_files.items[i]));
		if (!cmd_run_sync_and_reset(&cmd)) return Failed;
	}
	return Success;
}

static Result compile(Cpp_Program program)
{
	if (program.output_name == NULL)
		program.output_name = file_name_no_exts(program.source);
	switch (build.target_os)
	{
	case OS_MACOS:   return compile_macos(program);
	case OS_WINDOWS: return compile_windows(program);
	case OS_ANDROID: return compile_android(program);
	default:         return Failed;
	}
}

static Result write_object_from_binary_file(const char* output_file, const char* binary_file, const char* symbol_name)
{
	nob_log(INFO, "Generating binary object file %s -> %s", binary_file, output_file);
	
	String_Builder input = {0};
	if (!read_entire_file(binary_file, &input))
		return Failed;

	if (build.target_os == OS_ANDROID)
	{
		if (!mkdir_if_not_exists(output_file)) return Failed;
		if (write_object_from_binary_file_android_arm64(temp_sprintf("%s/arm64-v8a.o", output_file), input, symbol_name))
			return Failed;
		if (write_object_from_binary_file_android_armv7(temp_sprintf("%s/armeabi-v7a.o", output_file), input, symbol_name))
			return Failed;
		return Success;
	}

	String_Builder output = {0};

#if OS == OS_WINDOWS // x86_64 only

	const char* sym_start = temp_sprintf("%s_start", symbol_name);
	const char* sym_end   = temp_sprintf("%s_end", symbol_name);

    // COFF file header
    IMAGE_FILE_HEADER coff = {0};
    coff.Machine = IMAGE_FILE_MACHINE_AMD64;
    coff.NumberOfSections = 1;
    coff.TimeDateStamp = (uint32_t)time(NULL);
    coff.PointerToSymbolTable = sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_SECTION_HEADER) + input.count;
    coff.NumberOfSymbols = 3; // start, end, section symbol
	coff.Characteristics = IMAGE_FILE_DEBUG_STRIPPED;
	sb_append_buf(&output, &coff, sizeof(coff));

    // Section header (.rdata)
    IMAGE_SECTION_HEADER section = {0};
    memcpy(section.Name, ".rodata", 7);
    section.SizeOfRawData = input.count;
    section.PointerToRawData = sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_SECTION_HEADER);
    section.Characteristics = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_ALIGN_8BYTES;
	sb_append_buf(&output, &section, sizeof(section));

    // Section contents
	sb_append_buf(&output, input.items, input.count);

    // Write IMAGE_SYMBOL entries
    IMAGE_SYMBOL sym = {0};

    sym.N.Name.Short = 0;              // use string table
    sym.N.Name.Long = 4;               // offset into string table (after size field)
    sym.Value = 0;
    sym.SectionNumber = 1;
    sym.Type = IMAGE_SYM_DTYPE_POINTER << 8 | IMAGE_SYM_TYPE_BYTE;
    sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
	sb_append_buf(&output, &sym, sizeof(sym));

    sym.N.Name.Long = 4 + (uint32_t)(strlen(sym_start) + 1); // offset to second string
    sym.Value = input.count;
	sb_append_buf(&output, &sym, sizeof(sym));

    // Section symbol
    memset(&sym, 0, sizeof(sym));
    memcpy(sym.N.ShortName, ".rdata", 6);
    sym.SectionNumber = 1;
    sym.StorageClass = IMAGE_SYM_CLASS_STATIC;
	sb_append_buf(&output, &sym, sizeof(sym));

    // String table
    uint32_t strtab_size = sizeof(uint32_t) + strlen(sym_start) + strlen(sym_end) + 2;
	sb_append_buf(&output, &strtab_size, sizeof(strtab_size));
    sb_append_cstr(&output, sym_start);
    da_append(&output, 0);
    sb_append_cstr(&output, sym_end);
    da_append(&output, 0);

#elif OS == OS_MACOS // ARM64 only

    // Offsets
    size_t header_offset   = 0;
    size_t segment_offset  = header_offset + sizeof(struct mach_header_64);
    size_t section_offset  = segment_offset + sizeof(struct segment_command_64);
    size_t symtab_offset   = section_offset + sizeof(struct section_64);
    size_t version_offset  = symtab_offset + sizeof(struct symtab_command);
    size_t data_offset     = version_offset + sizeof(struct version_min_command);
    size_t sym_offset      = data_offset + input.count;
    size_t str_offset      = sym_offset + 2 * sizeof(struct nlist_64);

	const char* sym_start = temp_sprintf("_%s_start", symbol_name);
	const char* sym_end   = temp_sprintf("_%s_end", symbol_name);

    // --- Header ---
    struct mach_header_64 mh = {0};
    mh.magic      = MH_MAGIC_64;
    mh.cputype    = CPU_TYPE_ARM64;
    mh.cpusubtype = CPU_SUBTYPE_ARM64_ALL;
    mh.filetype   = MH_OBJECT;
    mh.ncmds      = 3; // segment + symtab + version
    mh.sizeofcmds = sizeof(struct segment_command_64) + sizeof(struct section_64)
                   + sizeof(struct symtab_command) + sizeof(struct version_min_command);
    mh.flags      = MH_SUBSECTIONS_VIA_SYMBOLS;
	sb_append_buf(&output, &mh, sizeof(mh));

    // --- Segment ---
    struct segment_command_64 sg = {0};
    sg.cmd     = LC_SEGMENT_64;
    sg.cmdsize = sizeof(sg) + sizeof(struct section_64);
    strcpy(sg.segname, "__DATA");
    sg.nsects  = 1;
    sg.maxprot = VM_PROT_READ;
    sg.initprot= VM_PROT_READ;
    sg.vmsize   = input.count;
    sg.filesize = input.count;
	sb_append_buf(&output, &sg, sizeof(sg));

    // --- Section ---
    struct section_64 sec = {0};
    strcpy(sec.sectname, "__binary");
    strcpy(sec.segname, "__DATA");
    sec.offset = data_offset;
    sec.size   = input.count;
    sec.align  = 2; // 4-byte alignment
	sb_append_buf(&output, &sec, sizeof(sec));

    // --- Symtab command ---
    struct symtab_command sc = {0};
    sc.cmd     = LC_SYMTAB;
    sc.cmdsize = sizeof(sc);
    sc.symoff  = sym_offset;
    sc.nsyms   = 2; // start + end
    sc.stroff  = str_offset;
    sc.strsize = 1 + strlen(sym_start) + 1 + strlen(sym_end) + 1;
	sb_append_buf(&output, &sc, sizeof(sc));

    // --- Version command ---
    struct version_min_command vmc = {0};
    vmc.cmd     = LC_VERSION_MIN_MACOSX;
    vmc.cmdsize = sizeof(vmc);
    vmc.version = 0x0A0F00; // macOS 10.15
    vmc.sdk     = 0x0A0F00;
	sb_append_buf(&output, &vmc, sizeof(vmc));

    // --- Data payload ---
	sb_append_buf(&output, input.items, input.count);

    // --- Symbol table ---
    struct nlist_64 syms[2] = {0};
    syms[0].n_un.n_strx = 1; // offset into string table
    syms[0].n_type      = N_SECT | N_EXT;
    syms[0].n_sect      = 1;
    syms[0].n_value     = 0; // start of section

    syms[1].n_un.n_strx = 1 + strlen(sym_start) + 1; // second string
    syms[1].n_type      = N_SECT | N_EXT;
    syms[1].n_sect      = 1;
    syms[1].n_value     = input.count; // end of section

	sb_append_buf(&output, &syms, sizeof(syms));

    // --- String table ---
    char *strtab = calloc(1, sc.strsize);
    strcpy(strtab + 1, sym_start);
    strcpy(strtab + 1 + strlen(sym_start) + 1, sym_end);
	sb_append_buf(&output, strtab, sc.strsize);
    free(strtab);
#endif

	if (!write_entire_file(output_file, output.items, output.count))
		return Failed;

	return Success;
}

int check_dependencies(void)
{
	cmd_append(&fission.include_dirs, "include");
	iterate (dependencies) {
        Dependency* d = &dependencies[i];
		if (!(d->targets & build.target_os)) continue;
        if (d->fetch(d)) return 1;
		cmd_append(&fission.include_dirs, d->include_path);
	}

	const char* header_only_output = temp_sprintf("%s/header_only%s", build.intermediate_dir, obj_ext(build.target_os));
	if (build.target_os == OS_ANDROID)
		header_only_output = temp_sprintf("%s/arm64-v8a.o", header_only_output);	
	const char* header_only_source = "src/header_only.cpp";
	if (needs_rebuild(header_only_output, &header_only_source, 1))
	{
		Cpp_Program header_only = {
			.source = header_only_source,
			.output_name = "header_only",
			.flags = COMPILE_OBJECT,
			.include_dirs = fission.include_dirs,
		};
		if (compile(header_only)) return Failed;
	}
	return 0;
}

int compile_shaders(void)
{
	File_Paths shader_files = {0};
	check(read_entire_dir("src/shaders", &shader_files));

	forn (shader_files.count) {
		const char* path = shader_files.items[i];
		if (strcmp(path, ".")  == 0) continue;
		if (strcmp(path, "..") == 0) continue;
		const char* name = file_name_no_exts(path);
		const char* binary_path = temp_sprintf("src/embed/%s.spv", name);
		const char* source_path = temp_sprintf("src/shaders/%s", path);
		if (!needs_rebuild(binary_path, &source_path, 1))
			continue;
		run(slang.compiler, "-target", "spirv", source_path, "-o", binary_path);
	}
	return 0;
}

int generate_embedded_objects(void)
{
	File_Paths embed_files = {0};
	check(read_entire_dir("src/embed", &embed_files));

	forn (embed_files.count) {
		const char* path = embed_files.items[i];
		if (strcmp(path, ".")  == 0) continue;
		if (strcmp(path, "..") == 0) continue;
		if (sv_end_with(sv_from_cstr(path), ".hpp")) continue;

		const char* name = file_name_no_exts(path);
		const char* output_path = temp_sprintf("%s/%s%s", build.intermediate_dir, path, obj_ext(build.target_os));
		const char* binary_path = temp_sprintf("src/embed/%s", path);
		const char* symbol_name = identifier_from_file_name(path);
		cmd_append(&fission.object_files, path);

		const char* full_output_path = output_path;
		if (build.target_os == OS_ANDROID)
			full_output_path = temp_sprintf("%s/arm64-v8a.o", output_path); // only check

		// Generate binary object
		if (needs_rebuild(full_output_path, &binary_path, 1))
		{
			if (write_object_from_binary_file(output_path, binary_path, symbol_name))
				return 1;
		}

		// Generate include file
		const char* header_path = temp_sprintf("src/embed/%s.hpp", path);
		if (!file_exists(header_path))
		{
			String_Builder builder = {0};
			sb_append_cstr(&builder, "namespace embedded\n{\n");
			sb_appendf(&builder, "\textern \"C\" const uint8_t %s_start[];\n", symbol_name);
			sb_appendf(&builder, "\textern \"C\" const uint8_t %s_end[];\n", symbol_name);
			sb_appendf(&builder, "}\n");
			check(write_entire_file(header_path, builder.items, builder.count));
		}
	}
	return 0;
}

//! NOTE: assumes called from "examples/" directory
Result fission_build_all_examples(Cpp_Program start)
{
	Cpp_Program program = start;
	
    File_Paths paths = {0};
    check(read_entire_dir(".", &paths));
    check(nob_set_current_dir(".."));
    for (int i = 0; i < paths.count; ++i)
    {
        String_View path = sv_from_cstr(paths.items[i]);
        if (!sv_end_with(path, ".cpp"))
            continue;
        
		program.source = temp_sprintf("examples/%s", paths.items[i]);
		if (compile(program)) return Failed;
    }
    return Success;
}

Result build_fission_all(void)
{
	if (build_fission()) return Failed;
	
	Cpp_Program start = {
		.include_dirs = fission.include_dirs,
		.flags = fission.flags & (~COMPILE_STATIC_LIBRARY),
	};
	
	cmd_append(&start.library_dirs, build.output_dir); 
	cmd_append(&start.libraries, "fission");
	
    iterate (dependencies) {
        Dependency* d = &dependencies[i];
		if (!(d->targets & build.target_os)) continue;
		cmd_append(&start.libraries, d->name);
		if (d->library_path)
			cmd_append(&start.library_dirs, d->library_path);
    }
	
	Result result = Success;
	scoped_timer("Build All Examples")
	{
		scoped_dir("examples")
		if (fission_build_all_examples(start))
			result = Failed;
	}
defer:
	return result;
}

Result build_fission(void)
{
	create_build_directories();
	check_cpp_compiler();
	if (check_dependencies()) return 1;
	if (compile_shaders()) return 1;
	if (generate_embedded_objects()) return 1;
	
	cmd_append(&fission.object_files, "header_only");
	
	Result r;
	scoped_timer("Build Fission")
	{
		r = compile(fission);
	}
	return r;
}

#endif // BUILD_H
