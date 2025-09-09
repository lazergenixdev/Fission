#include "Fission/os.hpp"

BEGIN_NAMESPACE(os)

auto os::init() -> Result {
	// Attach to the console only if run from commandline
	if (AttachConsole(ATTACH_PARENT_PROCESS) != 0)
	{
		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);
	}
	return Success;
}

END_NAMESPACE()
