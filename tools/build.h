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
#elif defined(__APPLE__) || defined(__MACH__)
#	include <TargetConditionals.h>
#	if TARGET_IPHONE_SIMULATOR == 1
#		define OS OS_IOS
#	elif TARGET_OS_IPHONE == 1
#		define OS OS_IOS
#	elif TARGET_OS_MAC == 1
#		define OS OS_MACOS
#	else
#		error "Unknown Apple platform!"
#	endif
#elif defined(__ANDROID__)
#	define OS OS_ANDROID
#elif defined(__linux__)
#	define OS OS_LINUX
#else
#	error "Unknown platform!"
#endif

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define PATH(L) "(\x1b[92m" L "\x1b[0m)"

#ifdef assert
#undef assert
#endif
void assert_impl(int cond, const char* info);
#define assert(E) assert_impl(E, #E)
#define check(E) if (!(E)) exit(1)
#define run(...) do { Cmd C = {0}; cmd_append(&C, __VA_ARGS__); check(cmd_run_sync(C)); cmd_free(C); } while(0)
#define len(A) (sizeof(A)/sizeof(A[0]))
#define forn(N) for (int i = 0; i < (N); ++i)
#define iterate(A) for (int i = 0; i < len(A); ++i)
#define create_symbolic_link(src,link) (symlink(src, link) != 0 ? nob_log(ERROR, "Failed to create symbolic link (%s -> %s)", link, src), 0 : 1)

#define scoped(start, end) for (int _i = (start, 0); _i < 1; (end), ++_i)
#define scoped_dir(dir) scoped(pushd(dir), popd())
#define scoped_log(level) for (int _old = minimal_log_level, _new = level; (minimal_log_level = _new), _new == level; _new = _old)
#define scoped_temp() for (int _i = (mkdir_if_not_exists("temp"), pushd("temp"), 0); _i < 1; (popd()), ++_i)
#define scoped_timer(what) for (uint64_t _start_ns = nanos_since_unspecified_epoch(), _done = 0; !_done; nob_log(INFO, what " took \x1b[93m%f\x1b[0m seconds", (double)((nanos_since_unspecified_epoch() - _start_ns)/1000)/1e6), _done = 1)

#if OS == OS_WINDOWS
#	define OBJ_EXT ".obj"
#else
#	define OBJ_EXT ".o"
#endif

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
    bool              header_only;
    const char*       subfolder;
};

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

struct {
	int         target_os;
	const char* output_dir;
	const char* intermediate_dir;
	const char* cache_dir;
} compiler;

bool compile(Cpp_Program program)
{
    Cmd cmd = {0};
	if (program.output_name == NULL)
		program.output_name = file_name_no_exts(program.source);
#if OS == OS_WINDOWS
	cmd_append(&cmd, "cl.exe", "/nologo", "/utf-8", "/std:c++20");
	cmd_append(&cmd, "/W4", "/EHsc-", "/MD"); //! TODO: compile with static CRT
	if (!(program.flags & COMPILE_DEBUG)) // Debug symbols are terrible with Optimizations
	switch (program.optimization) {
		default: cmd_append(&cmd, "/O2"); break;
		case Opt_None: break;
	}
	if ((program.flags & COMPILE_STATIC_LIBRARY) || (program.flags & COMPILE_OBJECT))
		cmd_append(&cmd, "/c");
    cmd_append(&cmd, program.source);
	forn (program.include_dirs.count)
		cmd_append(&cmd, temp_sprintf("/I%s", program.include_dirs.items[i]));
	cmd_append(&cmd, temp_sprintf("/Fo%s/%s.obj", compiler.intermediate_dir, program.output_name));
	if (program.flags & COMPILE_DEBUG)
	{
		cmd_append(&cmd, "/Zi");
		cmd_append(&cmd, temp_sprintf("/Fd%s/%s.pdb", compiler.output_dir, program.output_name));
	}
	if (!(program.flags & COMPILE_STATIC_LIBRARY) && !(program.flags & COMPILE_OBJECT))
	{	
		cmd_append(&cmd, temp_sprintf("/Fe%s/%s.exe", compiler.output_dir, program.output_name));
		cmd_append(&cmd, "/link", "/SUBSYSTEM:WINDOWS");
		forn (program.library_dirs.count)
			cmd_append(&cmd, temp_sprintf("/LIBPATH:\"%s\"", program.library_dirs.items[i]));
		forn (program.libraries.count)
			cmd_append(&cmd, temp_sprintf("%s.lib", program.libraries.items[i]));
	}
    if (!cmd_run_sync_and_reset(&cmd)) return 0;
	if (program.flags & COMPILE_STATIC_LIBRARY)
	{
		cmd_append(&cmd, "lib", "/nologo");
		cmd_append(&cmd, temp_sprintf("/OUT:%s/%s.lib", compiler.output_dir, program.output_name));
		cmd_append(&cmd, temp_sprintf("%s/%s.obj", compiler.intermediate_dir, program.output_name));
		forn (program.object_files.count)
			cmd_append(&cmd, temp_sprintf("%s/%s.obj", compiler.intermediate_dir, program.object_files.items[i]));
		if (!cmd_run_sync(cmd)) return 0;
	}
#else
	cmd_append(&cmd, "clang++", "-std=c++20");
	cmd_append(&cmd, "-Wall", "-Wextra", "-Wpedantic", "-fno-exceptions");
	switch (program.optimization) {
		default: cmd_append(&cmd, "-O2"); break;
		case Opt_None: break;
	}
	if ((program.flags & COMPILE_STATIC_LIBRARY) || (program.flags & COMPILE_OBJECT))
		cmd_append(&cmd, "-c");
	cmd_append(&cmd, program.source);
	forn (program.include_dirs.count)
		cmd_append(&cmd, temp_sprintf("-I%s", program.include_dirs.items[i]));

	cmd_append(&cmd, "-o", temp_sprintf("%s/%s.o", compiler.intermediate_dir, program.output_name));

	if (program.flags & COMPILE_DEBUG)
		cmd_append(&cmd, "-g");

	if (!(program.flags & COMPILE_STATIC_LIBRARY) && !(program.flags & COMPILE_OBJECT))
	{	
		cmd_append(&cmd, "-o", temp_sprintf("%s/%s", compiler.output_dir, program.output_name));
		forn (program.library_dirs.count)
			cmd_append(&cmd, temp_sprintf("-L%s", program.library_dirs.items[i]));
		forn (program.libraries.count)
			cmd_append(&cmd, temp_sprintf("-l%s", program.libraries.items[i]));
		// System Libraries
		cmd_append(&cmd, "-rpath", "@executable_path/");
		cmd_append(&cmd, "-framework", "OpenGL", "-framework", "Cocoa",
						 "-framework", "IOKit", "-framework", "CoreVideo");
	}
    if (!cmd_run_sync_and_reset(&cmd)) return 0;
	if (program.flags & COMPILE_STATIC_LIBRARY)
	{
		cmd_append(&cmd, "ar", "rvs");
		cmd_append(&cmd, temp_sprintf("%s/lib%s.a", compiler.output_dir, program.output_name));
		cmd_append(&cmd, temp_sprintf("%s/%s.o", compiler.intermediate_dir, program.output_name));
		forn (program.object_files.count)
			cmd_append(&cmd, temp_sprintf("%s/%s.o", compiler.intermediate_dir, program.object_files.items[i]));
		if (!cmd_run_sync(cmd)) return 0;
	}
#endif
	cmd_free(cmd);
	return true;
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

struct {
    const char **items;
    size_t count;
    size_t capacity;
} _directory_stack;

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

const char* cache_file_temp(const char* name)
{
	return temp_sprintf("%s/%s", compiler.cache_dir, name);
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

typedef enum {
	YES, NO
} Response;

Response ask(const char* question)
{
	printf("[\x1b[92minput\x1b[0m] %s (y/n) ", question);
	int ch = tolower(getchar());
	if (ch == '\n') return NO;
	while (getchar() != '\n');
	return ch == 'y' ? YES : NO;
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

int build_fission(void);
int build_fission_all(void);

#endif // BUILD_H
