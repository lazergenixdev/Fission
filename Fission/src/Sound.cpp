#include <Fission/Core/Sound.hh>

#ifdef FISSION_PLATFORM_WINDOWS
#include "Platform/Windows/SoundXAudio2.h"
#endif

namespace Fission
{

	ref<SoundEngine> SoundEngine::Create( const CreateInfo & info )
	{
		return make_ref<Platform::SoundEngineXAudio2>( info );
	}

}