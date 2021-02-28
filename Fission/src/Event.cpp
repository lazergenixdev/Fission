#include "LazerEngine/Core/Input/Event.h"

using namespace lazer;

IEventHandler * IEventHandler::Default() {
	static IEventHandler _DefEventHandler;
	return &_DefEventHandler;
}
