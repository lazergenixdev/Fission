#include <Fission/config.hpp>
#include <functional>

__FISSION_BEGIN__

struct Window_Create_Info {
	int width;
	int height;
	string title;
	struct Engine* engine;
};

__FISSION_END__