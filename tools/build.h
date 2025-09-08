#ifndef BUILD_H
#define BUILD_H

#if defined(_WIN32)
#	define PLATFORM_WINDOWS
#   define PLATFORM "windows"
#elif defined(__APPLE__) || defined(__MACH__)
#	include <TargetConditionals.h>
#	if TARGET_IPHONE_SIMULATOR == 1
#		define PLATFORM_IOS
#       define PLATFORM "ios"
#	elif TARGET_OS_IPHONE == 1
#		define PLATFORM_IOS
#       define PLATFORM "ios"
#	elif TARGET_OS_MAC == 1
#		define PLATFORM_MACOS
#       define PLATFORM "macos"
#	else
#		error "Unknown Apple platform!"
#	endif
#elif defined(__ANDROID__)
#	define PLATFORM_ANDROID
#   define PLATFORM "android"
#elif defined(__linux__)
#	define PLATFORM_LINUX
#   define PLATFORM "linux"
#else
#	error "Unknown platform!"
#endif

void assert_impl(int cond, const char* info);

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define PATH(L) "(\x1b[92m" L "\x1b[0m)"
#define CACHE_DIR "bin/" PLATFORM "/cache"

#ifdef assert
#undef assert
#endif
#define assert(E) assert_impl(E, #E)
#define check(E) if (!(E)) exit(1)
#define run(...) do { Cmd C = {0}; cmd_append(&C, __VA_ARGS__); check(cmd_run_sync(C)); cmd_free(C); } while(0)
#define len(A) (sizeof(A)/sizeof(A[0]))
#define forn(N) for (int i = 0; i < (N); ++i)
#define iterate(A) for (int i = 0; i < len(A); ++i)
#define def_struct(N) typedef struct N N; struct N
#define create_symbolic_link(src,link) (symlink(src, link) != 0 ? nob_log(ERROR, "Failed to create symbolic link (%s -> %s)", link, src), 0 : 1)

#define scoped(start, end) for (int i = (start, 0); i < 1; (end), ++i)
#define scoped_dir(dir) scoped(pushd(dir), popd())
#define scoped_log(level) for (int old = minimal_log_level, new = level; (minimal_log_level = new), new == level; new = old)

typedef struct Dependency Dependency;
typedef int (*P_fetch_callback)(Dependency*);

struct Dependency {
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

typedef enum {
	Opt_Speed  = 0, // speed is the default
	Opt_None   = 1,
} Optimization;

typedef enum {
	COMPILE_DEBUG          = (1<<0),
	COMPILE_STATIC_LIBRARY = (1<<1),
} Compile_Flags;

typedef struct {
	const char*    source;
	Cmd            include_dirs;
	Cmd            libraries;
	Cmd            library_dirs;
	const char*    output_name;
	Compile_Flags  flags;
	Optimization   optimization;
} Cpp_Program;

bool compile(Cpp_Program program)
{
    Cmd cmd = {0};
#if defined(PLATFORM_WINDOWS)
	cmd_append(&cmd, "cl.exe", "/nologo", "/utf-8", "/std:c++20");
	cmd_append(&cmd, "/W4", "/EHsc-", "/MD"); // TODO: compile with static CRT
	if (!(program.flags & COMPILE_DEBUG)) // Debug symbols are terrible with Optimizations
	switch (program.optimization) {
		default: cmd_append(&cmd, "/O2"); break;
		case Opt_None: break;
	}
	if (program.flags & COMPILE_STATIC_LIBRARY)
		cmd_append(&cmd, "/c");
    cmd_append(&cmd, program.source);
	forn (program.include_dirs.count)
		cmd_append(&cmd, temp_sprintf("/I%s", program.include_dirs.items[i]));
	if (program.output_name)
		cmd_append(&cmd, temp_sprintf("/Fo%s/%s.obj", "bin/" PLATFORM "/int", program.output_name));
	else
		cmd_append(&cmd, temp_sprintf("/Fo%s/", "bin/" PLATFORM "/int"));
	if (program.flags & COMPILE_DEBUG)
	{
		cmd_append(&cmd, "/Zi");
		if (program.output_name)
			cmd_append(&cmd, temp_sprintf("/Fd%s/%s.pdb", "bin/" PLATFORM, program.output_name));
		else
			cmd_append(&cmd, temp_sprintf("/Fd%s/", "bin/" PLATFORM));
	}
	if (!(program.flags & COMPILE_STATIC_LIBRARY))
	{	
		if (program.output_name)
			cmd_append(&cmd, temp_sprintf("/Fe%s/%s.exe", "bin/" PLATFORM, program.output_name));
		else
			cmd_append(&cmd, temp_sprintf("/Fe%s/", "bin/" PLATFORM));
		
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
		cmd_append(&cmd, temp_sprintf("/OUT:%s/%s.lib", "bin/" PLATFORM, program.output_name));
		cmd_append(&cmd, temp_sprintf("%s/%s.obj", "bin/" PLATFORM "/int", program.output_name));
		if (!cmd_run_sync(cmd)) return 0;
	}
	cmd_free(cmd);
#endif
	return true;
}

const char* library_temp(const char* dir, const char* name)
{
#if defined(PLATFORM_WINDOWS)
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
	return temp_sprintf(CACHE_DIR "/%s", name);
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

bool load_variable(const char** out, const char* name)
{
    const char* var = getenv(name);
    if (var == NULL) return 0;
    *out = var;
    return 1;
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
#if defined(PLATFORM_WINDOWS)
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
#else
#endif
}

const char* find_file_recursive(const char* search_path, const char* file)
{
	static char full_path[512];
	static int full_path_length = 0;
	
	const char* result = NULL;
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
	
	File_Paths children = {0};
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
	full_path_length -= k;
	return result;
}

void assert_impl(int cond, const char* info)
{
	if (cond == true) return;
	nob_log(ERROR, "Assertion Failed: %s", info);
	exit(1);
}

int build(void);
int build_all(void);

#endif // BUILD_H
