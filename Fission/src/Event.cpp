#include <Fission/Core/Input/Event.hh>

using namespace Fission;

EventHandler * EventHandler::Default() {
	static EventHandler _DefEventHandler;
	return &_DefEventHandler;
}
