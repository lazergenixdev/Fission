#include "../tools/build.h"

//! NOTE: assumes called from "examples/" directory
Result build_all_examples(Cpp_Program start)
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
