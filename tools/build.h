#ifndef BUILD_H
#define BUILD_H

#define OS_WINDOWS 0x01
#define OS_MACOS   0x02
#define OS_LINUX   0x04
#define OS_ANDROID 0x08
#define OS_IOS     0x10
#define OS_ALL     0xFF

#if defined(_WIN32)
#	define OS OS_WINDOWS
#elif defined(__APPLE__)
#	define OS OS_MACOS
#elif defined(__linux__)
#	define OS OS_LINUX
#else
#	error "Build program does not support this OS!"
#endif

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#if OS == OS_MACOS
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#endif
#include "android_elf.h"

#define PATH(L) "(\x1b[92m" L "\x1b[0m)"

#ifdef assert
#undef assert
#endif
void assert_impl(int cond, const char* info);
#define assert(E) assert_impl(E, #E)
#define check(E) if (!(E)) exit(1)
#define run(...) do { Cmd C = {0}; cmd_append(&C, __VA_ARGS__); check(cmd_run_sync(C)); cmd_free(C); } while(0)
#define run_output(output_path, ...) do { Cmd C = {0}; cmd_append(&C, __VA_ARGS__); check(cmd_run_opt(&C, (Cmd_Opt){.stdout_path = output_path, .stderr_path = output_path})); cmd_free(C); } while(0)
#define len(A) (sizeof(A)/sizeof(A[0]))
#define forn(N) for (int i = 0; i < (N); ++i)
#define iterate(A) for (int i = 0; i < len(A); ++i)

#define scoped(start, end) for (int _i = (start, 0); _i < 1; (end), ++_i)
#define scoped_dir(dir) scoped(pushd(dir), popd())
#define scoped_log(level) for (int _old = minimal_log_level, _new = level; (minimal_log_level = _new), _new == level; _new = _old)
#define scoped_temp() for (int _i = (mkdir_if_not_exists("temp"), pushd("temp"), 0); _i < 1; (popd()), ++_i)
#define scoped_timer(what) for (uint64_t _start_ns = nanos_since_unspecified_epoch(), _done = 0; !_done; nob_log(INFO, what " took \x1b[93m%f\x1b[0m seconds", (double)((nanos_since_unspecified_epoch() - _start_ns)/1000)/1e6), _done = 1)

#define MSVC_OPTIONS "/nologo", "/std:c++20", "/EHsc-", \
					 "/utf-8", "/GR-", "/MT", \
					 "/W4", "/wd4201"
#define CLANG_OPTIONS "-std=c++20", "-fno-exceptions", \
					  "-Wall", "-Wextra", "-Wpedantic", \
					  "-Wno-nested-anon-types", "-Wno-gnu-anonymous-struct", \
					  "-Wno-missing-designated-field-initializers"

#define X_ANDROID_ARCHITECTURES(X) \
	X("arm64-v8a",   "aarch64-linux-android35-clang++") \
	X("armeabi-v7a", "armv7a-linux-androideabi35-clang++") \

#if OS == OS_WINDOWS
#	define OBJ_EXT ".obj"
#	define SCRIPT_EXT ".bat"
#	define SCRIPT_COMMENT ":: "
#else
#	define OBJ_EXT ".o"
#	define SCRIPT_EXT ".sh"
#	define SCRIPT_COMMENT "# "
#endif

typedef enum { Success = 0, Failed = 1 } Result;

const char* home_path;
const char* android_sdk_path;
const char* android_toolchain; // compilers location
const char* android_tools;     // command-line tools
const char* android_platform;
const char* android_build_tools;

struct {
	int         target_os;
	const char* output_dir;
	const char* intermediate_dir;
	const char* cache_dir; // need? only used for vcvars
	const char* company;   // com.example (Android only)
	const char* keystore;  // (Android only)
} build;

Cmd cmd;

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

typedef enum {
	Opt_Speed  = 0, // speed is the default
	Opt_None   = 1,
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

Result build_fission(void);
Result build_fission_all(void);
void check_cpp_compiler(void);
void create_build_directories(void);
Result compile(Cpp_Program program);
Result write_object_from_binary_file(const char* output_file, const char* binary_file, const char* symbol_name);
const char* find_file_recursive(const char* search_path, const char* file);

// Example: "path/to/my/file.ext.ok" => "file.ext"
const char* file_name_no_exts(const char* path)
{
	int len = strlen(path);
	int start = len;
	for (;start > 0 && path[start-1] != '\\' && path[start-1] != '/'; --start);
	int end = start;
	for (;end < len && path[end] != '.'; ++end);
	return temp_sprintf("%.*s", end - start, path + start);
}

// Example: "path/to/my/file.ext.ok" => "path/to/my"
const char* path_parent(const char* path)
{
	int len = strlen(path);
	int start = len;
	for (;start > 0 && path[start-1] != '\\' && path[start-1] != '/'; --start);
	return temp_sprintf("%.*s", start, path);
}

const char* string_replace(const char* s, char from, char to)
{
	char* out = temp_strdup(s);
	for (int i = 0; out[i] != 0; ++i)
		if (out[i] == from)
			out[i] = to;
	return out;
}
bool string_ends_with(const char* str, const char* end)
{
	return sv_end_with(sv_from_cstr(str), end);
}

const char* identifier_from_file_name(const char* file_name)
{
	char* out = temp_strdup(file_name);
	for (int i = 0; file_name[i] != 0; ++i)
	{
		char ch = file_name[i];
		if (!('A' <= ch && ch <= 'Z')
		&&  !('a' <= ch && ch <= 'z')
		&&  !('0' <= ch && ch <= '9'))
			ch = '_';
		out[i] = ch;
	}
	return out;
}

const char* target_name(int os)
{
	switch (os) {
		case OS_WINDOWS: return "windows";
		case OS_MACOS:   return "macos";
		case OS_LINUX:   return "linux";
		case OS_ANDROID: return "android";
		case OS_IOS:     return "ios";
	}
	return NULL;
}

struct {
    const char **items;
    size_t count;
    size_t capacity;
} _directory_stack;

// Set current directory temporarily
void pushd(const char* path)
{
	const char* current = get_current_dir_temp();
	da_append(&_directory_stack, current);
	set_current_dir(path);
}
void popd()
{
	set_current_dir(da_last(&_directory_stack));
	_directory_stack.count -= 1;
}

struct {
    const char **items;
    size_t count;
    size_t capacity;
} _path_stack;

// Push onto $PATH temporarily
void pushp(const char* path)
{
	const char* current = getenv("PATH");
	da_append(&_path_stack, current);
	setenv("PATH", temp_sprintf("%s:%s", path, current), 1);
}
void popp()
{
	setenv("PATH", da_last(&_path_stack), 1);
	_path_stack.count -= 1;
}

void create_build_directories(void)
{
	scoped_log(WARNING) {
		check(mkdir_if_not_exists("bin"));
		build.output_dir = temp_sprintf("bin/%s", target_name(build.target_os));
		check(mkdir_if_not_exists(build.output_dir));
		build.intermediate_dir = temp_sprintf("%s/int", build.output_dir);
		check(mkdir_if_not_exists(build.intermediate_dir));
		build.cache_dir = temp_sprintf("%s/cache", build.output_dir);
		check(mkdir_if_not_exists(build.cache_dir));

		if (build.target_os == OS_ANDROID)
		{
			check(mkdir_if_not_exists(temp_sprintf("%s/lib", build.output_dir)));
			#define _MAKE_OUTPUT_DIR_ANDROID(ARCH, ...) \
				check(mkdir_if_not_exists(temp_sprintf("%s/lib/" ARCH, build.output_dir)));
			X_ANDROID_ARCHITECTURES(_MAKE_OUTPUT_DIR_ANDROID)
			#undef _MAKE_OUTPUT_DIR_ANDROID
		}
	}
}

void check_cpp_compiler_android(void)
{
	// Determine SDK location (look at default locations)
	const char* sdk_location = NULL;
#if OS == OS_WINDOWS
	const char* app_data = getenv("AppData");
	const char* path1 = temp_sprintf("%s/Local/Android/Sdk", app_data);
	const char* path2 = "C:/Program Files/Android/android-sdk";
	if (file_exists(path1)) sdk_location = path1;
	if (!sdk_location && file_exists(path2)) sdk_location = path2;
#elif OS == OS_MACOS
	const char* path1 = temp_sprintf("%s/Library/Android/sdk", home_path);
	if (file_exists(path1)) sdk_location = path1;
#elif OS == OS_LINUX
	const char* path1 = temp_sprintf("%s/Android/Sdk", home_path);
	if (file_exists(path1)) sdk_location = path1;
#endif
	if (!sdk_location) {
		// Install Android SDK
		nob_log(INFO, "Android SDK not found! Installing Android SDK...");
		exit(1); // TODO
	}
	else nob_log(INFO, "Found Android SDK " PATH("%s"), sdk_location);
	android_sdk_path = sdk_location;

	// Check/Install required tools (https://apilevels.com/)
	const char* sdkmanager = find_file_recursive(sdk_location, "sdkmanager");
	const char* sdkroot = temp_sprintf("--sdk_root=%s", sdk_location);
	scoped_dir(sdk_location)
	{
		if (!file_exists("build-tools/35.0.0"))
			run(sdkmanager, sdkroot, "--install", "build-tools;35.0.0");
		if (!file_exists("platforms/android-35"))
			run(sdkmanager, sdkroot, "--install", "platforms;android-35");
		if (!file_exists("ndk/28.2.13676358"))
			run(sdkmanager, sdkroot, "--install", "ndk;28.2.13676358");
		if (!file_exists("platform-tools"))
			run(sdkmanager, sdkroot, "--install", "platform-tools");
	}
	android_tools = path_parent(sdkmanager);
	android_platform = temp_sprintf("%s/platforms/android-35", sdk_location);
	android_build_tools = temp_sprintf("%s/build-tools/35.0.0", sdk_location);
	const char* ndk_path = temp_sprintf("%s/ndk/28.2.13676358/toolchains/llvm", sdk_location);
	const char* clang = find_file_recursive(ndk_path, "clang");
	const char* bin = path_parent(clang);
	nob_log(INFO, "Found Android toolchain " PATH("%s"), bin);
	android_toolchain = bin;

	// Check if there is a keystore available
	build.keystore = temp_sprintf("%s/.keystore", home_path);
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
		sb_appendf(&builder, "set PATH=%s/platform-tools;%PATH%\n", android_sdk_path);
	#else
		sb_appendf(&builder, "export PATH=\"%s/platform-tools:$PATH\"\n", android_sdk_path);
	#endif
		sb_appendf(&builder,
			SCRIPT_COMMENT "Start debugging by using `adb`\n"
			SCRIPT_COMMENT "`adb install test.apk`\n"
			SCRIPT_COMMENT "`adb shell am start -n dev.lazergenix.test/.MainActivity`\n"
			SCRIPT_COMMENT "`adb logcat`\n"
		);
		write_entire_file(setup_env, builder.items, builder.count);
	}
}

void check_cpp_compiler(void)
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

void cmd_append_all_ends_with(Cmd* cmd, const char* path, const char* end)
{
	File_Paths children = {0};
	if (!read_entire_dir(path, &children))
		return;
	forn (children.count) {
		const char* child = children.items[i];
		if (string_ends_with(child, end))
			cmd_append(cmd, temp_sprintf("%s/%s", path, child));
	}
}

Result compile_android(Cpp_Program program)
{
	Result result = Success;
	pushp(android_toolchain);
	Cmd options = {0};
	cmd_append(&options, CLANG_OPTIONS, "-fPIC"); // need PIC for objects?
	cmd_append(&options, program.source);
	forn (program.include_dirs.count)
		cmd_append(&options, temp_sprintf("-I%s", program.include_dirs.items[i]));
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

	const char* package = temp_sprintf("%s.%s", build.company, program.output_name);
	const char* namespace = string_replace(package, '.', '_');

	#define _COMPILE_ANDROID(ARCH, COMPILER, ...) \
		cmd_append(&cmd, COMPILER); \
		cmd_extend(&cmd, &options); \
		if (program.flags & (COMPILE_OBJECT|COMPILE_STATIC_LIBRARY)) \
			cmd_append(&cmd, "-o", temp_sprintf("%s/%s.o/" ARCH ".o", build.intermediate_dir, program.output_name)); \
		else { \
			cmd_append(&cmd, "src/platform_android.cpp"); \
			cmd_append(&cmd, temp_sprintf("-D__ANDROID_NAMESPACE__=%s", namespace)); \
			cmd_append(&cmd, temp_sprintf("-L%s/lib/" ARCH, build.output_dir)); \
			forn (program.libraries.count) \
				cmd_append(&cmd, temp_sprintf("-l%s", program.libraries.items[i])); \
			cmd_append(&cmd, "-landroid", "-llog", "-lvulkan"); \
			cmd_append(&cmd, "-o", temp_sprintf("%s/lib/" ARCH "/lib%s.so", build.output_dir, program.output_name)); \
		} \
		if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);
	X_ANDROID_ARCHITECTURES(_COMPILE_ANDROID)
	#undef _COMPILE_ANDROID

	if (program.flags & COMPILE_STATIC_LIBRARY)
	{
		#define _LIBRARY_ANDROID(ARCH, ...) \
		cmd_append(&cmd, "llvm-ar", "rvs"); \
		cmd_append(&cmd, temp_sprintf("%s/lib/" ARCH "/lib%s.a", build.output_dir, program.output_name)); \
		cmd_append(&cmd, temp_sprintf("%s/%s.o/" ARCH ".o", build.intermediate_dir, program.output_name)); \
		forn (program.object_files.count) { \
			const char* object_file = temp_sprintf("%s/%s.o", build.intermediate_dir, program.object_files.items[i]); \
			if (get_file_type(object_file) == FILE_DIRECTORY) \
				cmd_append(&cmd, temp_sprintf("%s/%s.o/" ARCH ".o", build.intermediate_dir, program.object_files.items[i])); \
			else \
				cmd_append(&cmd, object_file); \
		} \
		if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);
		X_ANDROID_ARCHITECTURES(_LIBRARY_ANDROID)
		#undef _LIBRARY_ANDROID
	}

	if (program.flags & (COMPILE_OBJECT|COMPILE_STATIC_LIBRARY))
		return_defer(Success);

	popp();
	pushp(android_build_tools);

	const char* package_path = string_replace(package, '.', '/');
	
	// Generate R.java
	cmd_append(&cmd, "aapt", "package", "-f", "-m");
	cmd_append(&cmd, "-S", "src/android/res", "-J", build.intermediate_dir);
	cmd_append(&cmd, "-M", "src/android/AndroidManifest.xml");
	cmd_append(&cmd, "-I", temp_sprintf("%s/android.jar", android_platform));
	cmd_append(&cmd, "--custom-package", package);
	if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);

	// Generate MainActivity with correct package declaration
	const char* activity_path = temp_sprintf("%s/%s/MainActivity.java", build.intermediate_dir, package_path);
	{
		String_Builder builder = {0};
		sb_appendf(&builder, "package %s.%s;\n\n", build.company, program.output_name);
		if (!read_entire_file("src/android/dev/lazergenix/fission/MainActivity.java", &builder))
			return_defer(Failed);
		if (!write_entire_file(activity_path, builder.items, builder.count))
			return_defer(Failed);
	}

	// Compile .java -> .class
	cmd_append(&cmd, "javac", "-classpath", temp_sprintf("%s/android.jar", android_platform));
	cmd_append(&cmd, "-d", build.intermediate_dir);
	cmd_append(&cmd, activity_path);
	cmd_append(&cmd, temp_sprintf("%s/%s/R.java", build.intermediate_dir, package_path));
	if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);

	// Generate classes.dex
	cmd_append(&cmd, "d8", "--min-api", "21");
	cmd_append(&cmd, "--classpath", temp_sprintf("%s/android.jar", android_platform));
	cmd_append_all_ends_with(&cmd, temp_sprintf("%s/%s", build.intermediate_dir, package_path), ".class");
	cmd_append(&cmd, "--output", build.intermediate_dir);
	if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);

	const char* output_apk = temp_sprintf("%s/%s.output.apk", build.intermediate_dir, program.output_name);

	// Generate APK
	cmd_append(&cmd, "aapt", "package", "-f");
	cmd_append(&cmd, "-M", "src/android/AndroidManifest.xml");
	cmd_append(&cmd, "-S", "src/android/res");
	cmd_append(&cmd, "-I", temp_sprintf("%s/android.jar", android_platform));
	cmd_append(&cmd, "-F", output_apk);
	cmd_append(&cmd, "--custom-package", package);
	if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);

	// Add classes.dex
	cmd_append(&cmd, "zip", "-qj", output_apk);
	cmd_append(&cmd, temp_sprintf("%s/classes.dex", build.intermediate_dir));
	if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);

	// Add shared libraries
	scoped_dir(build.output_dir)
	{
		cmd_append(&cmd, "zip", "-qr", temp_sprintf("int/%s.output.apk", program.output_name));
		#define _ADD_SHARED_LIBRARY(ARCH, ...) \
			cmd_append(&cmd, temp_sprintf("lib/" ARCH "/lib%s.so", program.output_name));
		X_ANDROID_ARCHITECTURES(_ADD_SHARED_LIBRARY)
		#undef _ADD_SHARED_LIBRARY
		if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);
	}

	// Align APK
	const char* unsigned_apk = temp_sprintf("%s/%s.unsigned.apk", build.intermediate_dir, program.output_name);
	cmd_append(&cmd, "zipalign", "-f", "4", output_apk, unsigned_apk);
	if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);

	// Sign APK
	cmd_append(&cmd, "apksigner", "sign", "--ks", build.keystore);
	cmd_append(&cmd, "--ks-key-alias", "debug");
	cmd_append(&cmd, "--ks-pass", "pass:android");
	cmd_append(&cmd, "--v1-signing-enabled", "true", "--v2-signing-enabled", "true");
	cmd_append(&cmd, "--out", temp_sprintf("%s/%s.apk", build.output_dir, program.output_name));
	cmd_append(&cmd, unsigned_apk);
	if (!cmd_run_sync_and_reset(&cmd)) return_defer(Failed);

defer:
	popp();
	return result;
}

Result compile_windows(Cpp_Program program)
{
	cmd_append(&cmd, "cl.exe", MSVC_OPTIONS);
	if (!(program.flags & COMPILE_DEBUG)) // Debug symbols are terrible with Optimizations
	switch (program.optimization) {
		default: cmd_append(&cmd, "/O2"); break;
		case Opt_None: cmd_append(&cmd, "/Od"); break;
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

Result compile_macos(Cpp_Program program)
{
	cmd_append(&cmd, "clang++", CLANG_OPTIONS);
	switch (program.optimization) {
		default: cmd_append(&cmd, "-O2"); break;
		case Opt_None: break;
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
		cmd_append(&cmd, "-o", temp_sprintf("%s/%s", build.output_dir, program.output_name));
		forn (program.library_dirs.count)
			cmd_append(&cmd, temp_sprintf("-L%s", program.library_dirs.items[i]));
		forn (program.libraries.count)
			cmd_append(&cmd, temp_sprintf("-l%s", program.libraries.items[i]));
		// System Libraries
		cmd_append(&cmd, "-rpath", "@executable_path/");
		cmd_append(&cmd, "-framework", "OpenGL", "-framework", "Cocoa",
						 "-framework", "IOKit", "-framework", "CoreVideo");
	}
    if (!cmd_run_sync_and_reset(&cmd)) return Failed;
	if (program.flags & COMPILE_STATIC_LIBRARY)
	{
		cmd_append(&cmd, "ar", "rvs");
		cmd_append(&cmd, temp_sprintf("%s/lib%s.a", build.output_dir, program.output_name));
		cmd_append(&cmd, temp_sprintf("%s/%s.o", build.intermediate_dir, program.output_name));
		forn (program.object_files.count)
			cmd_append(&cmd, temp_sprintf("%s/%s.o", build.intermediate_dir, program.object_files.items[i]));
		if (!cmd_run_sync_and_reset(&cmd)) return Failed;
	}
	return Success;
}

Result compile(Cpp_Program program)
{
	if (program.output_name == NULL)
		program.output_name = file_name_no_exts(program.source);
	switch (build.target_os)
	{
	case OS_MACOS: return compile_macos(program);
	case OS_WINDOWS: return compile_windows(program);
	case OS_ANDROID: return compile_android(program);
	default: return Failed;
	}
}

Result write_object_from_binary_file_android_armv7(const char* output_file, String_Builder input, const char* symbol_name)
{
	String_Builder output = {0};

    const char strtab_data[] = "\0.strtab\0.symtab\0.data\0.ARM.attributes";
	const char* sym_start = temp_sprintf("%s_start", symbol_name);
	const char* sym_end   = temp_sprintf("%s_end", symbol_name);
	uint32_t strtab_size = sizeof(strtab_data) + strlen(sym_start) + strlen(sym_end) + 2;

	// Symbol Table
	Elf32_Sym syms[3] = {0};
	syms[1].st_name = sizeof(strtab_data);
	syms[1].st_value = 0;
	syms[1].st_size = 0;
	syms[1].st_shndx = 3;
	syms[1].st_info = STB_GLOBAL << 4;
	syms[2].st_name = sizeof(strtab_data) + strlen(sym_start) + 1;
	syms[2].st_value = input.count;
	syms[2].st_size = 0;
	syms[2].st_shndx = 3;
	syms[2].st_info = STB_GLOBAL << 4;

    // ---- ELF Header ----
    Elf32_Ehdr ehdr = {0};
    memcpy(ehdr.e_ident, ELFMAG, SELFMAG);
    ehdr.e_ident[EI_CLASS] = ELFCLASS32;
    ehdr.e_ident[EI_DATA]  = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;

	ehdr.e_flags = 0x05000000;
    ehdr.e_type = ET_REL;              // relocatable
    ehdr.e_machine = EM_ARM;       // for Android ARM64
    ehdr.e_version = EV_CURRENT;
    ehdr.e_ehsize = sizeof(Elf32_Ehdr);
    ehdr.e_shentsize = sizeof(Elf32_Shdr);
    ehdr.e_shnum = 5;                  // 4 sections (null, .data, .shstrtab, .strtab)
    ehdr.e_shstrndx = 1;               // .shstrtab index
	ehdr.e_shoff = sizeof(ehdr) + sizeof(syms) + strtab_size + input.count
	             + sizeof(ehdr) + 8;

	sb_append_buf(&output, &ehdr, sizeof(ehdr));

    // section string table -> symbol table -> write data 
	sb_append_buf(&output, strtab_data, sizeof(strtab_data));
	sb_append_cstr(&output, sym_start);
	sb_append_null(&output);
	sb_append_cstr(&output, sym_end);
	sb_append_null(&output);
	sb_append_buf(&output, syms, sizeof(syms));
	sb_append_buf(&output, input.items, input.count);

	sb_append_buf(&output, &ehdr, sizeof(ehdr));
	sb_append_buf(&output, "\xff\xff\x00\xff\x00\xff\x00\xff", 8);

    // ---- Section headers ----
    Elf32_Shdr shdr_null = {0};
	sb_append_buf(&output, &shdr_null, sizeof(shdr_null));

    Elf32_Shdr strtab = {0}; // .shstrtab
    strtab.sh_name = 1; // offset in shstrtab
    strtab.sh_type = SHT_STRTAB;
    strtab.sh_offset = sizeof(ehdr);
    strtab.sh_size = strtab_size;
	strtab.sh_addralign = 1;
	sb_append_buf(&output, &strtab, sizeof(strtab));

    Elf32_Shdr symtab = {0}; // .symtab
    symtab.sh_name = strtab.sh_name + 7 + 1;
	symtab.sh_entsize = sizeof(syms[0]);
    symtab.sh_type = SHT_SYMTAB;
    symtab.sh_offset = strtab.sh_offset + strtab.sh_size;
    symtab.sh_size = sizeof(syms);
	symtab.sh_link = 1;
	symtab.sh_info = 1; // no idea why this is 1
	symtab.sh_addralign = 4;
	sb_append_buf(&output, &symtab, sizeof(symtab));

    Elf32_Shdr data = {0}; // .data
    data.sh_name = symtab.sh_name + 7 + 1;
    data.sh_type = SHT_PROGBITS;
    data.sh_flags = SHF_WRITE | SHF_ALLOC;
    data.sh_offset = symtab.sh_offset + symtab.sh_size;
    data.sh_size = input.count;
	data.sh_addralign = 4;
	sb_append_buf(&output, &data, sizeof(data));

    Elf32_Shdr arm_attributes = {0}; // .data
    arm_attributes.sh_name = data.sh_name + 5 + 1;
    arm_attributes.sh_type = 0x70000003; // https://github.com/ARM-software/abi-aa/blob/main/addenda32/addenda32.rst#3352the-target-related-attributes
    arm_attributes.sh_offset = data.sh_offset + data.sh_size;
    arm_attributes.sh_size = 60;
	arm_attributes.sh_addralign = 1;
	sb_append_buf(&output, &arm_attributes, sizeof(arm_attributes));

	if (!write_entire_file(output_file, output.items, output.count))
		return Failed;

	return Success;
}

Result write_object_from_binary_file_android_arm64(const char* output_file, String_Builder input, const char* symbol_name)
{
	String_Builder output = {0};

    const char strtab_data[] = "\0.strtab\0.symtab\0.data";
	const char* sym_start = temp_sprintf("%s_start", symbol_name);
	const char* sym_end   = temp_sprintf("%s_end", symbol_name);
	uint32_t strtab_size = sizeof(strtab_data) + strlen(sym_start) + strlen(sym_end) + 2;

	// Symbol Table
	Elf64_Sym syms[3] = {0};
	syms[1].st_name = sizeof(strtab_data);
	syms[1].st_value = 0;
	syms[1].st_size = 0;
	syms[1].st_shndx = 3;
	syms[1].st_info = STB_GLOBAL << 4;
	syms[2].st_name = sizeof(strtab_data) + strlen(sym_start) + 1;
	syms[2].st_value = input.count;
	syms[2].st_size = 0;
	syms[2].st_shndx = 3;
	syms[2].st_info = STB_GLOBAL << 4;

    // ---- ELF Header ----
    Elf64_Ehdr ehdr = {0};
    memcpy(ehdr.e_ident, ELFMAG, SELFMAG);
    ehdr.e_ident[EI_CLASS] = ELFCLASS64;
    ehdr.e_ident[EI_DATA]  = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;

    ehdr.e_type = ET_REL;              // relocatable
    ehdr.e_machine = EM_AARCH64;       // for Android ARM64
    ehdr.e_version = EV_CURRENT;
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_shentsize = sizeof(Elf64_Shdr);
    ehdr.e_shnum = 4;                  // 4 sections (null, .data, .shstrtab, .strtab)
    ehdr.e_shstrndx = 1;               // .shstrtab index
	ehdr.e_shoff = sizeof(ehdr) + sizeof(syms) + strtab_size + input.count;

	sb_append_buf(&output, &ehdr, sizeof(ehdr));

    // section string table -> symbol table -> write data 
	sb_append_buf(&output, strtab_data, sizeof(strtab_data));
	sb_append_cstr(&output, sym_start);
	sb_append_null(&output);
	sb_append_cstr(&output, sym_end);
	sb_append_null(&output);
	sb_append_buf(&output, syms, sizeof(syms));
	sb_append_buf(&output, input.items, input.count);

    // ---- Section headers ----
    Elf64_Shdr shdr_null = {0};
	sb_append_buf(&output, &shdr_null, sizeof(shdr_null));

    Elf64_Shdr strtab = {0}; // .shstrtab
    strtab.sh_name = 1; // offset in shstrtab
    strtab.sh_type = SHT_STRTAB;
    strtab.sh_offset = sizeof(ehdr);
    strtab.sh_size = strtab_size;
	strtab.sh_addralign = 1;
	sb_append_buf(&output, &strtab, sizeof(strtab));

    Elf64_Shdr symtab = {0}; // .symtab
    symtab.sh_name = 9; // offset in shstrtab
	symtab.sh_entsize = sizeof(syms[0]);
    symtab.sh_type = SHT_SYMTAB;
    symtab.sh_offset = strtab.sh_offset + strtab.sh_size;
    symtab.sh_size = sizeof(syms);
	symtab.sh_link = 1;
	symtab.sh_info = 1; // no idea why this is 1
	symtab.sh_addralign = 1;
	sb_append_buf(&output, &symtab, sizeof(symtab));

    Elf64_Shdr data = {0}; // .data
    data.sh_name = 9+8; // offset in shstrtab
    data.sh_type = SHT_PROGBITS;
    data.sh_flags = SHF_WRITE | SHF_ALLOC | SHF_RO_AFTER_INIT;
    data.sh_offset = symtab.sh_offset + symtab.sh_size;
    data.sh_size = input.count;
	data.sh_addralign = 4;
	sb_append_buf(&output, &data, sizeof(data));

	if (!write_entire_file(output_file, output.items, output.count))
		return Failed;

	return Success;
}

Result write_object_from_binary_file(const char* output_file, const char* binary_file, const char* symbol_name)
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

bool string_begins_with(const char* input, const char* begin)
{
	int m = 0;
	for (; input[m] == begin[m]; ++m);
	return begin[m] == 0;
}

const char* library_temp(const char* dir, const char* name)
{
#if OS == OS_WINDOWS
	return temp_sprintf("%s/%s.lib", dir, name);
#else
	return temp_sprintf("%s/lib%s.a", dir, name);
#endif
}

//! TODO: simplify caching API
const char* cache_file_temp(const char* name)
{
	return temp_sprintf("%s/%s", build.cache_dir, name);
}
const char* cache_find(const char* name)
{
	String_Builder builder = {0};
	if (read_entire_file(cache_file_temp(name), &builder))
	{
		sb_append_null(&builder);
		return builder.items;
	}
	return NULL;
}
void cache_set(const char* name, const char* value)
{
	check(write_entire_file(cache_file_temp(name), value, strlen(value)));
}

bool copy_file_if_not_exists(const char* src_path, const char* dst_path)
{
    if (file_exists(dst_path)) {
        nob_log(INFO, "file `%s` already exists", dst_path);
        return 1;
    }
    return copy_file(src_path, dst_path);
}

void root_run(const char* program, const char* args)
{
#if OS == OS_WINDOWS
#	pragma comment (lib, "shell32.lib")
	SHELLEXECUTEINFO sei = { sizeof(sei) };
	sei.lpVerb = "runas";
	sei.lpFile = program;
	sei.lpParameters = args;
	sei.nShow = 1;

	nob_log(INFO, "CMD: runas %s %s", program, args);
	if (!ShellExecuteExA(&sei)) {
		DWORD err = GetLastError();
		if (err == ERROR_CANCELLED) {
			nob_log(ERROR, "User refused elevation.");
		} else {
			nob_log(ERROR, "Failed to launch, error %lu\n", err);
		}
		exit(1);
	}
#endif
}

const char* find_file_recursive(const char* search_path, const char* file)
{
	static char full_path[512];
	static int full_path_length = 0;
	
	const char* result = NULL;
	File_Paths children = {0};
	int k = 0;
	
	// Add to full path
	for (; search_path[k] != 0; ++k) {
		full_path[full_path_length++] = search_path[k];
	}
	assert(full_path_length < sizeof(full_path));
	full_path[full_path_length] = 0; // null terminate
	
	File_Type type = get_file_type(full_path);
	assert(type != -1);
	if (type != FILE_DIRECTORY)
		return_defer(NULL);
	
	k += 1;
	full_path[full_path_length++] = '/'; // add separator
	assert(full_path_length < sizeof(full_path));
	full_path[full_path_length] = 0; // null terminate
	
	check(read_entire_dir(full_path, &children));
	forn (children.count) {
		const char* child = children.items[i];
		if (strcmp(child, ".")  == 0) continue;
		if (strcmp(child, "..") == 0) continue;
		if (strcmp(child, file) == 0)
			return_defer(temp_sprintf("%.*s%s", full_path_length, full_path, child));
		
		const char* found = find_file_recursive(child, file);
		if (found != NULL)
			return_defer(found);
	}
defer:
	da_free(children); // strings stored in temp memory
	full_path_length -= k;
	return result;
}
const char* find_begins_with_in_directory(const char* path, const char* begin)
{
	const char* result = NULL;
	File_Paths children = {0};
	check(read_entire_dir(path, &children));
	forn (children.count) {
		const char* child = children.items[i];
		if (string_begins_with(child, begin))
			return_defer(child);
	}
defer:
	da_free(children); // strings stored in temp memory
	return result;
}

const char* find_any_in_directory(const char* path)
{
	const char* result = NULL;
	File_Paths children = {0};
	if (!read_entire_dir(path, &children))
		return_defer(NULL);
	forn (children.count) {
		const char* child = children.items[i];
		if (strcmp(child, ".")  == 0) continue;
		if (strcmp(child, "..") == 0) continue;
		return_defer(child);
	}
defer:
	da_free(children); // strings stored in temp memory
	return result;
}

//! @param n: how many digits to include (Example: n=2 => "1.2")
//! @returns: length of the resulting string
int truncate_version(const char* version, int n)
{
    int i = 0, k = 0;
    while (version[i] != 0) {
        if (version[i] == '.') k += 1;
        if (k >= n) break;
        i += 1;
    }
    return i;
}

void assert_impl(int cond, const char* info)
{
	if (cond == true) return;
	nob_log(ERROR, "Assertion Failed: %s", info);
	exit(1);
}

#endif // BUILD_H
