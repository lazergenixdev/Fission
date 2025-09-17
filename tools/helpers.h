#include "android_elf.h"

#if OS == OS_MACOS
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#elif OS == OS_WINDOWS
#pragma comment (lib, "shell32.lib")
#endif

#define INFO_PLIST_HEADER \
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" \
		"<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n" \
		"<plist version=\"1.0\">\n"

#define INFO_PLIST_FOOTER \
		"</plist>\n"

#define VULKAN_ICD_MACOS \
	"{\n" \
	"    \"file_format_version\" : \"1.0.0\",\n" \
	"    \"ICD\": {\n" \
	"        \"library_path\": \"../../../Frameworks/libMoltenVK.dylib\",\n" \
	"        \"api_version\" : \"1.3.0\",\n" \
	"        \"is_portability_driver\" : true\n" \
	"    }\n" \
	"}\n"

	
#if OS == OS_WINDOWS
#	define ANDROID_COMPILER(X) X ".cmd" // thanks Android, very cool
#	define ANDROID_D8 "d8.bat" // I am disappointed
#	define ANDROID_APKSIGNER "apksigner.bat" // ._.
#	define OBJ_EXT ".obj"
#	define SCRIPT_EXT ".ps1"
#	define setenv(name, value) _putenv_s(name, value)
#	define PATH_SEP ";"
#else
#	define ANDROID_COMPILER(X) "./" X
#	define ANDROID_D8 "./d8"
#	define ANDROID_APKSIGNER "./apksigner"
#	define OBJ_EXT ".o"
#	define SCRIPT_EXT ".sh"
#	define setenv(name, value) setenv(name, value, 1)
#	define PATH_SEP ":"
#endif

#ifdef assert
#undef assert
#endif
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

#define PATH(L) "(\x1b[92m" L "\x1b[0m)"

typedef enum {
	Success = 0,
	Failed  = 1,
} Result;

static void assert_impl(int cond, const char* info)
{
	if (cond == true) return;
	nob_log(ERROR, "Assertion Failed: %s", info);
	__builtin_trap();
}

static const char* string_replace(const char* s, char from, char to)
{
	char* out = temp_strdup(s);
	for (int i = 0; out[i] != 0; ++i)
		if (out[i] == from)
			out[i] = to;
	return out;
}
static bool string_ends_with(const char* str, const char* end)
{
	return sv_end_with(sv_from_cstr(str), end);
}
static bool string_begins_with(const char* input, const char* begin)
{
	int m = 0;
	for (; input[m] == begin[m]; ++m);
	return begin[m] == 0;
}

// Example: "path/to/my/file.ext.ok" => "file.ext"
static const char* file_name_no_exts(const char* path)
{
	int len = strlen(path);
	int start = len;
	for (;start > 0 && path[start-1] != '\\' && path[start-1] != '/'; --start);
	int end = start;
	for (;end < len && path[end] != '.'; ++end);
	return temp_sprintf("%.*s", end - start, path + start);
}

// Example: "path/to/my/file.ext.ok" => "path/to/my"
static const char* path_parent(const char* path)
{
	int len = strlen(path);
	int start = len - 1;
	for (;start > 0 && path[start] != '\\' && path[start] != '/'; --start);
	return temp_sprintf("%.*s", start, path);
}

static const char* identifier_from_file_name(const char* file_name)
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

static const char* home_directory()
{
	static const char* path = NULL;
	if (path == NULL)
		path = getenv(OS == OS_WINDOWS? "HOMEPATH" : "HOME");
	assert(path != NULL);
	return path;
}

static const char* os_name(int os)
{
	switch (os) {
		case OS_WINDOWS: return "windows";
		case OS_MACOS:   return "macos";
		case OS_LINUX:   return "linux";
		case OS_ANDROID: return "android";
		case OS_IOS:     return "ios";
	}
	assert_impl(false, "OS not valid");
	return NULL;
}

static const char* obj_ext(int os)
{
	switch (os) {
		case OS_WINDOWS: return ".obj";
		default:         return ".o";
	}
}

static const char* library_temp(const char* dir, const char* name)
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
} _directory_stack = {0};

// Set current directory temporarily
static void pushd(const char* path)
{
	const char* current = get_current_dir_temp();
	da_append(&_directory_stack, current);
	set_current_dir(path);
}
static void popd()
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
//! NOTE: does not work well on windows, need more testing
static void pushp(const char* path)
{
	const char* current = getenv("PATH");
	da_append(&_path_stack, current);
	setenv("PATH", temp_sprintf("%s" PATH_SEP "%s", path, current));
}
static void popp()
{
	setenv("PATH", da_last(&_path_stack));
	_path_stack.count -= 1;
}

static void cmd_append_all_ends_with(Cmd* cmd, const char* path, const char* end)
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

static bool copy_file_if_not_exists(const char* src_path, const char* dst_path)
{
    if (file_exists(dst_path)) {
        nob_log(INFO, "file `%s` already exists", dst_path);
        return 1;
    }
    return copy_file(src_path, dst_path);
}

static const char* find_file_recursive(const char* search_path, const char* file)
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

static const char* find_begins_with_in_directory(const char* path, const char* begin)
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

static const char* find_any_in_directory(const char* path)
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

static bool mkdir_recursive(const char* path)
{
	int len = strlen(path);
	char* p = (char*)path;
	int ends_with_slash = 0;
	int begin = 1;
	forn (len) {
		if (!(p[i] == '/' || p[i] == '\\'))
		{
			ends_with_slash = 0;
			begin = 0;
			continue;
		}
		if (begin) continue;
		p[i] = '\0';
		if (!mkdir_if_not_exists(p))
			return false;
		p[i] = '/';
		ends_with_slash = 1;
	}
	if (ends_with_slash)
		return mkdir_if_not_exists(p);
	return true;
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

void root_run(const char* program, const char* args)
{
#if OS == OS_WINDOWS
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

static Result write_object_from_binary_file_android_armv7(const char* output_file, String_Builder input, const char* symbol_name)
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

static Result write_object_from_binary_file_android_arm64(const char* output_file, String_Builder input, const char* symbol_name)
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
