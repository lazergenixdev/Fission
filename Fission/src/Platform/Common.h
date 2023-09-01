#include <Fission/config.hpp>
#include <Fission/Base/String.hpp>
#include <Fission/Core/Window.hh>
#include <functional>

__FISSION_BEGIN__

struct Window_Create_Info {
	int width;
	int height;
	string title;
	Window_Mode mode;
	u32 display_index;
};

__FISSION_END__