#include "../tools/build.h"

/*
#define I(a,b) a ## b
#define cmd_cpp_compiler(cmd) cmd_append(cmd, "clang++")
#define cmd_cpp_flags(cmd) cmd_append(cmd, "-std=c++20", "-Wall", "-Wextra", "-rpath", "@executable_path/")
#define cmd_cpp_include(cmd, F) cmd_append(cmd, temp_sprintf("-I%s", F))
#define cmd_cpp_link(cmd, L) cmd_append(cmd, temp_sprintf("-l%s", L))
#define cmd_cpp_library(cmd, L) cmd_append(cmd, temp_sprintf("-L%s", L))
#define cmd_cpp_output(cmd, N) cmd_append(cmd, "-o", N)
#define cmd_cpp_dependency(cmd, d) do {                                       \
    if ((d).name) cmd_cpp_link(cmd, (d).name);                                \
    if ((d).include_path) cmd_cpp_include(cmd, (d).include_path);             \
    if ((d).library_path) cmd_cpp_library(cmd, (d).library_path); } while(0)
#define cmd_cpp_platform(cmd) cmd_append(cmd, "-framework", "OpenGL", "-framework", "Cocoa", "-framework", "IOKit", "-framework", "CoreVideo")
*/

/*
void build_example(const char* source)
{
    Cmd cmd = {0};
    cmd_cpp_compiler(&cmd);
    cmd_cpp_flags(&cmd);
    cmd_cpp_platform(&cmd);
//    iterate (dependencies) cmd_cpp_dependency(&cmd, dependencies[i]);
    cmd_append(&cmd, temp_sprintf("examples/%s", source));
    cmd_cpp_output(&cmd, temp_sprintf("bin/" PLATFORM "/%s", name(source)));
    check(cmd_run_sync(cmd));
    cmd_free(cmd);
}
*/

int build_all_examples(Cpp_Program start)
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
		if (!compile(program)) return 1;
    }
    return 0;
}
