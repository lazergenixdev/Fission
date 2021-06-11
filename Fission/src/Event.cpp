#include <Fission/Core/Input/Event.hh>

using namespace Fission;

IFEventHandler * IFEventHandler::Default() {
	static IFEventHandler _DefEventHandler;
	return &_DefEventHandler;
}
