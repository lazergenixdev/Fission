#include "build.h"

int main(int argc, char* argv[])
{
#ifndef DEBUG
	NOB_GO_REBUILD_URSELF_PLUS(argc, argv,
		"build.h", "tools/core_build.h", "dependencies.h");
#endif
	Result r = Success;
	scoped_timer("Build")
	{
		enum {
			Build_Fission = 0,
			Build_All     = 1,
		} action = Build_Fission;
		build.target_os = OS;
		build.company = "dev.lazergenix";

		forn (argc) {
			if (strcmp(argv[i], "all") == 0) action = Build_All;
			if (strcmp(argv[i], "debug") == 0) fission.flags |= COMPILE_DEBUG;
			if (strcmp(argv[i], "android") == 0) build.target_os = OS_ANDROID;
		}
		
		switch (action)
		{
			case Build_All: r = build_fission_all(); break;
			default:        r = build_fission(); break;
		}
		log_result(r);
	}
	return r;
}
