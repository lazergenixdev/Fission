#include "Fission/os.hpp"

BEGIN_NAMESPACE(os)

#if defined(OS_WINDOWS)
auto init() -> bool {
	// Attach to the console only if run from commandline
	if (AttachConsole(ATTACH_PARENT_PROCESS) != 0)
	{
		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);
	}
	return false;
}
#else
auto init() -> bool
{
	return false;
}
#endif

END_NAMESPACE()
