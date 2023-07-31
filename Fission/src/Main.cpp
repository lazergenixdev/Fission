#include <Fission/Core/Engine.hh>
#include "Version.h"

#if PROFILE
#define PROFILER_IMPLEMENTATION
#include "profiler.hpp"
std::unique_ptr<profiler::Session> session;
#endif

fs::Engine engine {
	.version = {FISSION_VERSION_MAJ,FISSION_VERSION_MIN,FISSION_VERSION_PAT},
};

using namespace fs;

FISSION_MAIN_FUNCTION()
{
	platform::Instance instance = { FISSION_MAIN_ARGS };

	if (engine.create(instance, on_create()))
		return 1;

	engine.run();

	if (engine.destroy())
		return 1;

	return 0;
}
