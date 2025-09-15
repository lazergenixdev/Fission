#include "build.h"

int main(int argc, char* argv[])
{
#ifndef DEBUG
    NOB_GO_REBUILD_URSELF_PLUS(argc, argv,
		"build.h", "tools/helpers.h", "dependencies.h");
#endif

	enum {
		Build_Fission = 0,
		Build_All     = 1,
		Build_Clean   = 2, //! TODO: remove all generated files
	} action = Build_Fission;
	build.target_os = OS;
	build.company = "dev.lazergenix";

	forn (argc) {
		if (strcmp(argv[i], "all") == 0) action = Build_All;
		if (strcmp(argv[i], "debug") == 0) fission.flags |= COMPILE_DEBUG;
		if (strcmp(argv[i], "android") == 0) build.target_os = OS_ANDROID;
	}
	
	Result r = 0;
	switch (action)
	{
		case Build_All: r = build_fission_all(); break;
		default:        r = build_fission(); break;
	}
	nob_log(INFO, r? "\x1b[91mCompilation Failed!\x1b[0m" : "\x1b[92mCompilation Succedded!\x1b[0m");
	return r;
}
