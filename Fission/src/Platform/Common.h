#include <Fission/config.hpp>
#include <Fission/Base/String.hpp>
#include <Fission/Core/Display.hh>
#include <functional>

__FISSION_BEGIN__

struct Window_Create_Info {
	int width;
	int height;
	string title;
	Window_Mode mode;
	u32 display_index;
	struct Engine* engine;
};

__FISSION_END__