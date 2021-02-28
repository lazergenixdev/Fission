#include "Fission/Core/Input/Event.h"

using namespace Fission;

IEventHandler * IEventHandler::Default() {
	static IEventHandler _DefEventHandler;
	return &_DefEventHandler;
}
